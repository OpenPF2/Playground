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
	/**
	 * An input component for directing player actions to whichever character is currently being controlled.
	 */
	UPROPERTY(BlueprintReadOnly)
	UInputComponent* ControlledCharacterInputComponent;

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
	UFUNCTION(BlueprintCallable, Category="OpenPF2 Playground|Characters")
	void AcknowledgeOwnership(TScriptInterface<IPF2CharacterInterface> InCharacter) const;
};
