// Copyright 2021-2022 Guy Elsmore-Paddock. All Rights Reserved.
// Adapted from content that is Copyright Epic Games, Inc. (Third Person Sample).
// Licensed only for use with Unreal Engine.

#pragma once

#include <Camera/CameraComponent.h>

#include <GameFramework/SpringArmComponent.h>

#include "InputBindableCharacterInterface.h"
#include "PF2CharacterBase.h"
#include "OpenPF2PlaygroundCharacterBase.generated.h"

// =====================================================================================================================
// Forward Declarations (to minimize header dependencies)
// =====================================================================================================================
class UPF2CommandBindingsComponent;

// =====================================================================================================================
// Normal Declarations
// =====================================================================================================================
/**
 * Base class for all characters in the OpenPF2 Playground sample.
 */
UCLASS(Config=Game)
// ReSharper disable once CppClassCanBeFinal
class AOpenPF2PlaygroundCharacterBase : public APF2CharacterBase, public IInputBindableCharacterInterface
{
	GENERATED_BODY()

protected:
	/**
	 * Camera boom positioning the camera behind the character.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	USpringArmComponent* CameraBoom;

	/**
	 * Camera that follows the player in third-person when outside of encounters.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	UCameraComponent* FollowCamera;

	/**
	 * Component that enables character abilities to be bound to input in a dynamic/configurable way at run-time.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	UPF2CommandBindingsComponent* AbilityBindings;

	/**
	 * Base turn rate, in deg/sec. Other scaling may affect final turn rate.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/**
	 * Base look up/down rate, in deg/sec. Other scaling may affect final rate.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

public:
	/**
	 * Constructs a new instance.
	 */
	AOpenPF2PlaygroundCharacterBase();

	// =================================================================================================================
	// Public Methods - IInputBindableCharacterInterface Implementation
	// =================================================================================================================
	UFUNCTION()
	virtual void LoadInputActionBindings() override;

	virtual void SetupClientAbilityChangeListener() override;

	// =================================================================================================================
	// Public Methods
	// =================================================================================================================
	/**
	 * Gets the camera boom sub-object.
	 *
	 * @return
	 *	The camera boom.
	 */
	FORCEINLINE USpringArmComponent* GetCameraBoom() const
	{
		return this->CameraBoom;
	}

	/**
	 * Gets the "follow camera" sub-object.
	 *
	 * This is a camera that provides the player with a view behind the third-person character.
	 *
	 * @return
	 *	The camera.
	 */
	FORCEINLINE UCameraComponent* GetFollowCamera() const
	{
		return this->FollowCamera;
	}

protected:
	// =================================================================================================================
	// Protected Methods - APawn Overrides
	// =================================================================================================================
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	// =================================================================================================================
	// Protected Native Event Callbacks
	// =================================================================================================================
	/**
	 * Called for forwards/backward input.
	 */
	void Native_OnMoveForwardBack(float Value);

	/**
	 * Called for side to side, strafing input.
	 */
	void Native_OnMoveRightLeft(float Value);

	/**
	 * Called via input to turn at a given rate.
	 *
	 * @param Rate
	 *	The normalized rate of turn (i.e., 1.0 means 100% of desired turn rate).
	 */
	void Native_OnTurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate.
	 *
	 * @param Rate
	 *	The normalized rate of turn (i.e., 1.0 means 100% of desired turn rate).
	 */
	void Native_OnLookUpAtRate(float Rate);

	/**
	 * Handler for when a touch input begins.
	 */
	void Native_OnTouchStarted(ETouchIndex::Type FingerIndex, FVector Location);

	/**
	 * Handler for when a touch input stops.
	 */
	void Native_OnTouchStopped(ETouchIndex::Type FingerIndex, FVector Location);

	/**
	 * Resets HMD orientation in VR.
	 */
	void Native_OnResetVR();
};
