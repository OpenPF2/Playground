// Copyright 2021 Guy Elsmore-Paddock. All Rights Reserved.
// Adapted from content that is Copyright Epic Games, Inc. (Third Person Sample).
// Licensed only for use with Unreal Engine.

#pragma once

#include <CoreMinimal.h>
#include <GameFramework/Character.h>

#include "OpenPF2PlaygroundCharacterBase.generated.h"

/**
 * Base class for all characters in the OpenPF2 Playground sample.
 */
UCLASS(Config=Game)
// ReSharper disable once CppClassCanBeFinal
class AOpenPF2PlaygroundCharacterBase : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

public:
	AOpenPF2PlaygroundCharacterBase();

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

protected:
	// =================================================================================================================
	// Protected Methods - APawn Overrides
	// =================================================================================================================
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

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
