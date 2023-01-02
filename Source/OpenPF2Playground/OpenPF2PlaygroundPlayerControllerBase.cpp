// OpenPF2 for UE Game Logic, Copyright 2022, Guy Elsmore-Paddock. All Rights Reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
// distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include "OpenPF2PlaygroundPlayerControllerBase.h"

#include <HeadMountedDisplayFunctionLibrary.h>

#include <GameFramework/Character.h>

#include "InputBindableCharacterInterface.h"
#include "PF2CharacterInterface.h"

AOpenPF2PlaygroundPlayerControllerBase::AOpenPF2PlaygroundPlayerControllerBase()
{
	// set our turn rates for input
	this->BaseTurnRate   = 45.0f;
	this->BaseLookUpRate = 45.0f;
}

void AOpenPF2PlaygroundPlayerControllerBase::SetupInputComponent()
{
	UInputComponent* Input;

	Super::SetupInputComponent();

	Input = this->InputComponent;

	this->ControlledCharacterInputComponent =
		NewObject<UInputComponent>(this, UInputComponent::StaticClass(), TEXT("PC_ControlledCharacterInput"));

	this->ControlledCharacterInputComponent->RegisterComponent();
	this->PushInputComponent(this->ControlledCharacterInputComponent);

	if (Input != nullptr)
	{
		check(Input);

		// Set up gameplay key bindings
		Input->BindAction("Jump", IE_Pressed, this, &AOpenPF2PlaygroundPlayerControllerBase::Native_OnJump);
		Input->BindAction("Jump", IE_Released, this, &AOpenPF2PlaygroundPlayerControllerBase::Native_OnStopJumping);

		// We do not consume input for these mappings so that the bird's eye camera pawn (flying camera) can handle
		// its own input, to give the player the ability to move the camera around.
		this->BindAxisWithPassthrough<AOpenPF2PlaygroundPlayerControllerBase>(
			Input,
			"MoveForwardBack",
			&AOpenPF2PlaygroundPlayerControllerBase::Native_OnMoveForwardBack
		);

		this->BindAxisWithPassthrough<AOpenPF2PlaygroundPlayerControllerBase>(
			Input,
			"MoveRightLeft",
			&AOpenPF2PlaygroundPlayerControllerBase::Native_OnMoveRightLeft
		);

		// We have 2 versions of the rotation bindings to handle two distinct kinds of devices:
		//	- "Turn" handles devices that provide an absolute delta, such as a mouse.
		//	- "TurnRate" is for devices that we choose to treat as a rate of change, such as an analog joystick.
		Input->BindAxis("Turn", this, &AOpenPF2PlaygroundPlayerControllerBase::Native_OnTurn);
		this->BindAxisWithPassthrough<AOpenPF2PlaygroundPlayerControllerBase>(
			Input,
			"TurnRate",
			&AOpenPF2PlaygroundPlayerControllerBase::Native_OnTurnAtRate
		);

		Input->BindAxis("LookUp", this, &AOpenPF2PlaygroundPlayerControllerBase::Native_OnLookUp);
		this->BindAxisWithPassthrough<AOpenPF2PlaygroundPlayerControllerBase>(
			Input,
			"LookUpRate",
			&AOpenPF2PlaygroundPlayerControllerBase::Native_OnLookUpAtRate
		);

		// handle touch devices
		Input->BindTouch(IE_Pressed, this, &AOpenPF2PlaygroundPlayerControllerBase::Native_OnTouchStarted);
		Input->BindTouch(IE_Released, this, &AOpenPF2PlaygroundPlayerControllerBase::Native_OnTouchStopped);

		// VR headset functionality
		Input->BindAction("ResetVR", IE_Pressed, this, &AOpenPF2PlaygroundPlayerControllerBase::Native_OnResetVR);
	}
}

void AOpenPF2PlaygroundPlayerControllerBase::Native_OnCharacterGiven(
	const TScriptInterface<IPF2CharacterInterface>& GivenCharacter)
{
	Super::Native_OnCharacterGiven(GivenCharacter);

	this->AcknowledgeOwnership(GivenCharacter);
}

void AOpenPF2PlaygroundPlayerControllerBase::AcknowledgeOwnership(
	const TScriptInterface<IPF2CharacterInterface> InCharacter) const
{
	IInputBindableCharacterInterface* BindableCharacterIntf =
		Cast<IInputBindableCharacterInterface>(InCharacter.GetObject());

	// Ensure the ASC has been initialized.
	InCharacter->InitializeOrRefreshAbilities();

	if (BindableCharacterIntf != nullptr)
	{
		BindableCharacterIntf->LoadInputActionBindings();
		BindableCharacterIntf->SetupClientAbilityChangeListener();
	}
}

bool AOpenPF2PlaygroundPlayerControllerBase::GetHitResultForScreenPosition(const FVector2D         InPosition,
                                                                           const ECollisionChannel InTraceChannel,
                                                                           const bool              bInTraceComplex,
                                                                           FHitResult&             OutHitResult) const
{
	const bool bHit = this->GetHitResultAtScreenPosition(InPosition, InTraceChannel, bInTraceComplex, OutHitResult);

	if (!bHit)
	{
		// If there was no hit we reset the results. This is redundant but helps Blueprint users
		OutHitResult = FHitResult();
	}

	return bHit;
}

FVector2D AOpenPF2PlaygroundPlayerControllerBase::GetCenterOfViewport() const
{
	FVector2D                  CenterPosition;
	const ULocalPlayer*        LocalPlayer    = this->GetLocalPlayer();
	const UGameViewportClient* ViewportClient = LocalPlayer->ViewportClient;

	if ((LocalPlayer != nullptr) && (ViewportClient != nullptr))
	{
		FVector2D ViewportSize;

		ViewportClient->GetViewportSize(ViewportSize);

		CenterPosition = FVector2D(
			ViewportSize.X / 2.0f,
			ViewportSize.Y / 2.0f
		);
	}

	return CenterPosition;
}

// ReSharper disable once CppMemberFunctionMayBeConst
void AOpenPF2PlaygroundPlayerControllerBase::Native_OnJump()
{
	ACharacter* PossessedCharacter = this->GetCharacter();

	if (PossessedCharacter != nullptr)
	{
		PossessedCharacter->Jump();
	}
}

// ReSharper disable once CppMemberFunctionMayBeConst
void AOpenPF2PlaygroundPlayerControllerBase::Native_OnStopJumping()
{
	ACharacter* PossessedCharacter = this->GetCharacter();

	if (PossessedCharacter != nullptr)
	{
		PossessedCharacter->StopJumping();
	}
}

// ReSharper disable once CppMemberFunctionMayBeConst
void AOpenPF2PlaygroundPlayerControllerBase::Native_OnMoveForwardBack(const float Value)
{
	ACharacter* PossessedCharacter = this->GetCharacter();

	if ((PossessedCharacter != nullptr) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = this->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		PossessedCharacter->AddMovementInput(Direction, Value);
	}
}

// ReSharper disable once CppMemberFunctionMayBeConst
void AOpenPF2PlaygroundPlayerControllerBase::Native_OnMoveRightLeft(const float Value)
{
	ACharacter* PossessedCharacter = this->GetCharacter();

	if ((PossessedCharacter != nullptr) && (Value != 0.0f))
	{
		// find out which way is right
		const FRotator Rotation = this->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get right vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement in that direction
		PossessedCharacter->AddMovementInput(Direction, Value);
	}
}

// ReSharper disable once CppMemberFunctionMayBeConst
void AOpenPF2PlaygroundPlayerControllerBase::Native_OnTurn(const float Value)
{
	ACharacter* PossessedCharacter = this->GetCharacter();

	if (PossessedCharacter != nullptr)
	{
		PossessedCharacter->AddControllerYawInput(Value);
	}
}

// ReSharper disable once CppMemberFunctionMayBeConst
void AOpenPF2PlaygroundPlayerControllerBase::Native_OnTurnAtRate(const float Rate)
{
	ACharacter* PossessedCharacter = this->GetCharacter();

	if (PossessedCharacter != nullptr)
	{
		// Calculate delta for this frame from the rate information
		PossessedCharacter->AddControllerYawInput(Rate * this->BaseTurnRate * this->GetWorld()->GetDeltaSeconds());
	}
}

// ReSharper disable once CppMemberFunctionMayBeConst
void AOpenPF2PlaygroundPlayerControllerBase::Native_OnLookUp(const float Value)
{
	ACharacter* PossessedCharacter = this->GetCharacter();

	if (PossessedCharacter != nullptr)
	{
		PossessedCharacter->AddControllerPitchInput(Value);
	}
}

// ReSharper disable once CppMemberFunctionMayBeConst
void AOpenPF2PlaygroundPlayerControllerBase::Native_OnLookUpAtRate(const float Rate)
{
	ACharacter* PossessedCharacter = this->GetCharacter();

	if (PossessedCharacter != nullptr)
	{
		// Calculate delta for this frame from the rate information
		PossessedCharacter->AddControllerPitchInput(Rate * this->BaseLookUpRate * this->GetWorld()->GetDeltaSeconds());
	}
}

void AOpenPF2PlaygroundPlayerControllerBase::Native_OnTouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
	this->Native_OnJump();
}

void AOpenPF2PlaygroundPlayerControllerBase::Native_OnTouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
	this->Native_OnStopJumping();
}

// ReSharper disable once CppMemberFunctionMayBeStatic
void AOpenPF2PlaygroundPlayerControllerBase::Native_OnResetVR()
{
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}
