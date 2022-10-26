// Copyright 2021-2022 Guy Elsmore-Paddock. All Rights Reserved.
// Adapted from content that is Copyright Epic Games, Inc. (Third Person Sample)
// Licensed only for use with Unreal Engine.

#include "OpenPF2PlaygroundCharacterBase.h"

#include <HeadMountedDisplayFunctionLibrary.h>

#include <Camera/CameraComponent.h>

#include <Components/CapsuleComponent.h>
#include <Components/InputComponent.h>

#include <GameFramework/CharacterMovementComponent.h>
#include <GameFramework/Controller.h>
#include <GameFramework/SpringArmComponent.h>

#include "OpenPF2Playground.h"

#include "Commands/PF2CommandBindingsComponent.h"

#include "Utilities/PF2LogUtilities.h"

AOpenPF2PlaygroundCharacterBase::AOpenPF2PlaygroundCharacterBase()
{
	// Set size for collision capsule
	this->GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);

	// set our turn rates for input
	this->BaseTurnRate   = 45.0f;
	this->BaseLookUpRate = 45.0f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	this->bUseControllerRotationPitch = false;
	this->bUseControllerRotationYaw   = false;
	this->bUseControllerRotationRoll  = false;

	UCharacterMovementComponent* CharacterMovementComponent = this->GetCharacterMovement();

	// Configure character movement
	CharacterMovementComponent->bOrientRotationToMovement = true; // Character moves in the direction of input...
	CharacterMovementComponent->RotationRate              = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	CharacterMovementComponent->JumpZVelocity             = 600.0f;
	CharacterMovementComponent->AirControl                = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	USpringArmComponent* CameraBoomComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));

	CameraBoomComponent->SetupAttachment(RootComponent);
	CameraBoomComponent->TargetArmLength         = 300.0f; // The camera follows at this distance behind the character
	CameraBoomComponent->bUsePawnControlRotation = true;   // Rotate the arm based on the controller

	this->CameraBoom = CameraBoomComponent;

	// Create a follow camera
	UCameraComponent* FollowCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));

	// Attach the camera to the end of the boom and let the boom adjust to match the controller orientation.
	FollowCameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);

	// Camera does not rotate relative to arm
	FollowCameraComponent->bUsePawnControlRotation = false;

	FollowCamera = FollowCameraComponent;

	// Create the component that allows binding abilities to input actions.
	UPF2CommandBindingsComponent* BindingsComponent =
		CreateDefaultSubobject<UPF2CommandBindingsComponent>("AbilityBindings");

	this->AbilityBindings = BindingsComponent;

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character)
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)
}

void AOpenPF2PlaygroundCharacterBase::LoadInputActionBindings()
{
	UE_LOG(
		LogPf2PlaygroundInput,
		Verbose,
		TEXT("[%s] Character ('%s') is loading activatable abilities."),
		*(PF2LogUtilities::GetHostNetId(this->GetWorld())),
		*(this->GetIdForLogs())
	);

	this->AbilityBindings->ClearBindings();

	if (this->AbilitySystemComponent != nullptr)
	{
		this->AbilityBindings->LoadAbilitiesFromCharacter();
	}
}

void AOpenPF2PlaygroundCharacterBase::SetupClientAbilityChangeListener()
{
	IPF2AbilitySystemInterface* Asc = Cast<IPF2AbilitySystemInterface>(this->GetAbilitySystemComponent());

	if (Asc == nullptr)
	{
		UE_LOG(
			LogPf2PlaygroundInput,
			Warning,
			TEXT("[%s] Character ('%s') is missing an OpenPF2-compatible ASC and cannot listen for ability changes."),
			*(PF2LogUtilities::GetHostNetId(this->GetWorld())),
			*(this->GetIdForLogs())
		);
	}
	else
	{
		Asc->GetClientAbilityChangeDelegate()->AddUniqueDynamic(
			this,
			&AOpenPF2PlaygroundCharacterBase::LoadInputActionBindings
		);
	}
}

void AOpenPF2PlaygroundCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAxis("MoveForwardBack", this, &AOpenPF2PlaygroundCharacterBase::Native_OnMoveForwardBack);
	PlayerInputComponent->BindAxis("MoveRightLeft", this, &AOpenPF2PlaygroundCharacterBase::Native_OnMoveRightLeft);

	// We have 2 versions of the rotation bindings to handle two distinct kinds of devices:
	//   - "Turn" handles devices that provide an absolute delta, such as a mouse.
	//   - "TurnRate" is for devices that we choose to treat as a rate of change, such as an analog joystick.
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AOpenPF2PlaygroundCharacterBase::Native_OnTurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AOpenPF2PlaygroundCharacterBase::Native_OnLookUpAtRate);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &AOpenPF2PlaygroundCharacterBase::Native_OnTouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &AOpenPF2PlaygroundCharacterBase::Native_OnTouchStopped);

	// VR headset functionality
	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &AOpenPF2PlaygroundCharacterBase::Native_OnResetVR);

	this->AbilityBindings->ConnectToInput(PlayerInputComponent);
}

void AOpenPF2PlaygroundCharacterBase::Native_OnMoveForwardBack(const float Value)
{
	const AController* PlayerController = this->Controller;

	if ((PlayerController != nullptr) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = PlayerController->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		this->AddMovementInput(Direction, Value);
	}
}

void AOpenPF2PlaygroundCharacterBase::Native_OnMoveRightLeft(float Value)
{
	const AController* PlayerController = this->Controller;

	if ((PlayerController != nullptr) && (Value != 0.0f))
	{
		// find out which way is right
		const FRotator Rotation = PlayerController->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get right vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement in that direction
		this->AddMovementInput(Direction, Value);
	}
}

void AOpenPF2PlaygroundCharacterBase::Native_OnTurnAtRate(const float Rate)
{
	// Calculate delta for this frame from the rate information
	this->AddControllerYawInput(Rate * this->BaseTurnRate * this->GetWorld()->GetDeltaSeconds());
}

void AOpenPF2PlaygroundCharacterBase::Native_OnLookUpAtRate(const float Rate)
{
	// Calculate delta for this frame from the rate information
	this->AddControllerPitchInput(Rate * this->BaseLookUpRate * this->GetWorld()->GetDeltaSeconds());
}

void AOpenPF2PlaygroundCharacterBase::Native_OnTouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
	this->Jump();
}

void AOpenPF2PlaygroundCharacterBase::Native_OnTouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
	this->StopJumping();
}

// ReSharper disable once CppMemberFunctionMayBeStatic
void AOpenPF2PlaygroundCharacterBase::Native_OnResetVR()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}
