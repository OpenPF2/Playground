// Copyright 2021-2023 Guy Elsmore-Paddock. All Rights Reserved.
// Adapted from content that is Copyright Epic Games, Inc. (Third Person Sample)
// Licensed only for use with Unreal Engine.

#include "OpenPF2PlaygroundCharacterBase.h"

#include <Camera/CameraComponent.h>

#include <Components/CapsuleComponent.h>
#include <Components/InputComponent.h>

#include <GameFramework/CharacterMovementComponent.h>
#include <GameFramework/Controller.h>
#include <GameFramework/SpringArmComponent.h>

#include "OpenPF2Playground.h"

#include "Commands/PF2AbilityBindingsComponent.h"

#include "Utilities/PF2LogUtilities.h"

AOpenPF2PlaygroundCharacterBase::AOpenPF2PlaygroundCharacterBase()
{
	// Set size for collision capsule
	this->GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);

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

	this->FollowCamera = FollowCameraComponent;

	// Create the component that allows binding abilities to input actions.
	UPF2AbilityBindingsComponent* BindingsComponent =
		CreateDefaultSubobject<UPF2AbilityBindingsComponent>("AbilityBindings");

	// Allow Player Controller or Character to still react to bound inputs.
	BindingsComponent->SetConsumeInput(false);

	this->AbilityBindings = BindingsComponent;

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character)
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)
}

void AOpenPF2PlaygroundCharacterBase::LoadInputAbilityBindings()
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
			&AOpenPF2PlaygroundCharacterBase::LoadInputAbilityBindings
		);
	}
}

TScriptInterface<IPF2AbilityBindingsInterface> AOpenPF2PlaygroundCharacterBase::GetAbilityBindingsComponent() const
{
	return this->AbilityBindings;
}

void AOpenPF2PlaygroundCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	this->AbilityBindings->ConnectToInput(PlayerInputComponent);
}
