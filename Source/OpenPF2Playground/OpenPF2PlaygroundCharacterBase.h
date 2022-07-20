// Copyright 2021 Guy Elsmore-Paddock. All Rights Reserved.
// Adapted from content that is Copyright Epic Games, Inc. (Third Person Sample).
// Licensed only for use with Unreal Engine.

#pragma once

#include <CoreMinimal.h>
#include "PF2CharacterBase.h"

#include "OpenPF2PlaygroundCharacterBase.generated.h"

/**
 * Base class for all characters in the OpenPF2 Playground sample.
 */
UCLASS(Config=Game)
// ReSharper disable once CppClassCanBeFinal
class AOpenPF2PlaygroundCharacterBase : public APF2CharacterBase
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

	/**
	 * Component for granting abilities to the character that are bound to input.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UPF2CommandBindingsComponent* AbilityBindings;

public:
	AOpenPF2PlaygroundCharacterBase();

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

	// =================================================================================================================
	// Public Methods - APawn Overrides
	// =================================================================================================================
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_Controller() override;

protected:
	// =================================================================================================================
	// Protected Methods - APawn Overrides
	// =================================================================================================================
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	// =================================================================================================================
	// Protected Methods
	// =================================================================================================================
	/**
	 * Resets HMD orientation in VR.
	 */
	void OnResetVR();

	/**
	 * Called for forwards/backward input.
	 */
	void MoveForward(float Value);

	/**
	 * Called for side to side, strafing input.
	 */
	void MoveRight(float Value);

	/**
	 * Called via input to turn at a given rate.
	 *
	 * @param Rate
	 *	The normalized rate of turn (i.e., 1.0 means 100% of desired turn rate).
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate.
	 *
	 * @param Rate
	 *	The normalized rate of turn (i.e., 1.0 means 100% of desired turn rate).
	 */
	void LookUpAtRate(float Rate);

	/**
	 * Handler for when a touch input begins.
	 */
	void TouchStarted(ETouchIndex::Type FingerIndex, FVector Location);

	/**
	 * Handler for when a touch input stops.
	 */
	void TouchStopped(ETouchIndex::Type FingerIndex, FVector Location);

	/**
	 * Loads default input action bindings from all of the granted, activatable abilities of this character.
	 *
	 * If this character has any abilities already bound to input, those bindings are cleared before the input is bound.
	 * If the input bindings component is already wired-up to input for this character, the actions are bound to input
	 * actions immediately.
	 *
	 * This should be called on the client whenever the abilities of the character have changed.
	 */
	void LoadInputActivatableAbilities();

public:
	// =================================================================================================================
	// Public Methods
	// =================================================================================================================
	/**
	 * Gets the camera boom sub-object.
	 *
	 * @return
	 *	The camera boom.
	 */
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const
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
	FORCEINLINE class UCameraComponent* GetFollowCamera() const
	{
		return this->FollowCamera;
	}
};
