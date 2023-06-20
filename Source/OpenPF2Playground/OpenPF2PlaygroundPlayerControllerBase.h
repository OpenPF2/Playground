// OpenPF2 for UE Game Logic, Copyright 2022-2023, Guy Elsmore-Paddock. All Rights Reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
// distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include "PF2PlayerControllerBase.h"
#include "OpenPF2PlaygroundPlayerControllerBase.generated.h"

// =====================================================================================================================
// Forward Declarations (to minimize header dependencies)
// =====================================================================================================================
class UEnhancedInputComponent;

// =====================================================================================================================
// Normal Declarations
// =====================================================================================================================
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
	UEnhancedInputComponent* ControlledCharacterInputComponent;

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
	UFUNCTION(BlueprintCallable, NetMulticast, Reliable, Category="OpenPF2 Playground|Player Controllers")
	void Multicast_DisableAutomaticCameraManagement();

	/**
	 * Resume having this player controller manage the camera target during the possession of pawns.
	 */
	UFUNCTION(BlueprintCallable, NetMulticast, Reliable, Category="OpenPF2 Playground|Player Controllers")
	void Multicast_EnableAutomaticCameraManagement();
};
