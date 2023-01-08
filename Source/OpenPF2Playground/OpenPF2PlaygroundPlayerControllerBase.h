// OpenPF2 for UE Game Logic, Copyright 2022, Guy Elsmore-Paddock. All Rights Reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
// distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include "PF2PlayerControllerBase.h"
#include "OpenPF2PlaygroundPlayerControllerBase.generated.h"

/**
 * Base class for OpenPF2 Playground player controllers.
 */
UCLASS()
// ReSharper disable once CppClassCanBeFinal
class OPENPF2PLAYGROUND_API AOpenPF2PlaygroundPlayerControllerBase : public APF2PlayerControllerBase
{
	GENERATED_BODY()

protected:
	// =================================================================================================================
	// Protected Fields
	// =================================================================================================================
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

	/**
	 * An input component for directing player actions to whichever character is currently being controlled.
	 */
	UPROPERTY(BlueprintReadOnly)
	UInputComponent* ControlledCharacterInputComponent;

public:
	// =================================================================================================================
	// Public Constructors
	// =================================================================================================================
	/**
	 * Constructs a new instance.
	 */
	explicit AOpenPF2PlaygroundPlayerControllerBase();

protected:
	// =================================================================================================================
	// Protected Methods - APlayerController Overrides
	// =================================================================================================================
	virtual void SetupInputComponent() override;

	// =================================================================================================================
	// Protected Methods - APF2PlayerControllerBase Overrides
	// =================================================================================================================
	virtual void Native_OnCharacterGiven(const TScriptInterface<IPF2CharacterInterface>& GivenCharacter) override;

	// =================================================================================================================
	// Protected Methods
	// =================================================================================================================
	/**
	 * Acknowledges that this player controller owns/can control the specified character.
	 *
	 * Unlike AcknowledgePossession(), this is called as soon as a character has been given to this player controller
	 * rather than only when a character is being possessed by this player controller. This is important for two
	 * reasons:
	 * 1) While in Exploration mode, the player only possesses (controls) a single character at a time while AI
	 *    controllers possess the rest of the player's party.
	 * 2) While in Encounter mode, the player does not possess any characters at all. Instead, all characters are
	 *    possessed by AI controllers and the player issues commands to the AI controllers to direct the characters.
	 *
	 * Without this, the player would have to take control of each member of their party at least once during
	 * Exploration mode to ensure that the characters were initialized prior to Encounter mode.
	 *
	 * @param InCharacter
	 *	The character of which this player controller is taking ownership.
	 */
	UFUNCTION(BlueprintCallable, Category="OpenPF2 Playground|Player Controllers")
	void AcknowledgeOwnership(TScriptInterface<IPF2CharacterInterface> InCharacter) const;

	/**
	 * Binds the specified axis to the specified callback in a way that does not consume input after being invoked.
	 *
	 * @param Input
	 *	The input component to which input will be bound.
	 * @param AxisName
	 *	The name of the input axis.
	 * @param Func
	 *	The callback to invoke when input is received.
	 *
	 * @return
	 *	A reference to the binding for the axis. The reference is only guaranteed to be valid until another axis is
	 *	bound.
	 */
	template<class UserClass>
	FInputAxisBinding& BindAxisWithPassthrough(
		UInputComponent* Input,
		const FName AxisName,
		const typename FInputAxisHandlerSignature::TUObjectMethodDelegate<UserClass>::FMethodPtr Func)
	{
		FInputAxisBinding& Binding = Input->BindAxis(AxisName, this, Func);

		Binding.bConsumeInput = false;

		return Binding;
	}

	/**
	 * Performs a collision query on a trace channel using a specific point in screen space.
	 *
	 * The upper-left corner of the screen is (0, 0). Positive X numbers move further right, while positive Y numbers
	 * move further down.
	 *
	 * @param InPosition
	 *	The position to project into the game world.
	 * @param InTraceChannel
	 *	The channel on which to perform the trace.
	 * @param bInTraceComplex
	 *	Whether to perform the trace against complex collision or simple collision.
	 * @param OutHitResult
	 *	The hit result.
	 *
	 * @return
	 *	Whether the collision trace was successful (intersected world geometry).
	 */
	UFUNCTION(BlueprintCallable, Category="OpenPF2 Playground|Player Controllers", meta=(bInTraceComplex=true))
	bool GetHitResultForScreenPosition(
		UPARAM(DisplayName="Position")
		const FVector2D InPosition,

		UPARAM(DisplayName="Trace Channel")
		const ECollisionChannel InTraceChannel,

		UPARAM(DisplayName="Trace Complex")
		const bool bInTraceComplex,

		UPARAM(DisplayName="Hit Result")
		FHitResult& OutHitResult) const;

	/**
	 * Gets a vector representing the position (in pixels) of the center of the game viewport.
	 *
	 * @return
	 *	A vector representing the center of the screen.
	 */
	UFUNCTION(BlueprintCallable, Category="OpenPF2 Playground|Player Controllers")
	FVector2D GetCenterOfViewport() const;

	/**
	 * Stop this player controller from managing the camera target during the possession of pawns.
	 *
	 * This allows camera targets to be managed manually.
	 */
	UFUNCTION(BlueprintCallable, Category="OpenPF2 Playground|Player Controllers")
	void DisableAutomaticCameraManagement();

	/**
	 * Resume having this player controller manage the camera target during the possession of pawns.
	 */
	UFUNCTION(BlueprintCallable, Category="OpenPF2 Playground|Player Controllers")
	void EnableAutomaticCameraManagement();

	// =================================================================================================================
	// Protected Native Event Callbacks
	// =================================================================================================================
	/**
	 * Input callback for making the character jump.
	 */
	void Native_OnJump();

	/**
	 * Input callback for making the character stop jumping.
	 */
	void Native_OnStopJumping();

	/**
	 * Input callback for forward and backward movement.
	 *
	 * @param Value
	 *	The amount to move the character forward or backward.
	 */
	void Native_OnMoveForwardBack(float Value);

	/**
	 * Input callback for side-to-side, strafing movement.
	 *
	 * @param Value
	 *	The amount to move the character right or left.
	 */
	void Native_OnMoveRightLeft(float Value);

	/**
	 * Input callback to turn the camera by a certain amount.
	 *
	 * This is used for devices that provide an absolute delta, such as a mouse.
	 *
	 * @param Value
	 *	The amount to turn the camera left or right.
	 */
	void Native_OnTurn(float Value);

	/**
	 * Input callback to turn the camera at the given rate.
	 *
	 * This is used for devices that we choose to treat as a rate of change, such as an analog joystick.
	 *
	 * @param Rate
	 *	The normalized rate of turn (i.e., 1.0 means 100% of desired turn rate) for moving the camera left or right.
	 */
	void Native_OnTurnAtRate(float Rate);

	/**
	 * Input callback to look the camera up or down by a certain amount.
	 *
	 * This is used for devices that provide an absolute delta, such as a mouse.
	 *
	 * @param Value
	 *	The amount to turn the camera up or down.
	 */
	void Native_OnLookUp(float Value);

	/**
	 * Input callback to look up or down at the given rate.
	 *
	 * This is used for devices that we choose to treat as a rate of change, such as an analog joystick.
	 *
	 * @param Rate
	 *	The normalized rate of turn (i.e., 1.0 means 100% of desired turn rate) for moving the camera up or down.
	 */
	void Native_OnLookUpAtRate(float Rate);

	/**
	 * Input callback for the start of a touch input.
	 *
	 * @param FingerIndex
	 *	Which finger touched the screen.
	 * @param Location
	 *	Where on the screen the finger touched the screen.
	 */
	void Native_OnTouchStarted(ETouchIndex::Type FingerIndex, FVector Location);

	/**
	 * Input callback for the end of a touch input.
	 *
	 * @param FingerIndex
	 *	Which finger touched the screen.
	 * @param Location
	 *	Where on the screen the finger touched the screen.
	 */
	void Native_OnTouchStopped(ETouchIndex::Type FingerIndex, FVector Location);

	/**
	 * Input callback to reset HMD orientation in VR.
	 */
	void Native_OnResetVR();
};
