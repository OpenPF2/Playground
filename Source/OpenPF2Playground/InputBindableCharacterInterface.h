// OpenPF2 for UE Game Logic, Copyright 2022-2023, Guy Elsmore-Paddock. All Rights Reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
// distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include <UObject/Interface.h>

#include "InputBindableCharacterInterface.generated.h"

// =====================================================================================================================
// Normal Declarations
// =====================================================================================================================
UINTERFACE(MinimalAPI, BlueprintType, meta=(CannotImplementInterfaceInBlueprint))
class UInputBindableCharacterInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * An interface for characters that support binding actions to input.
 */
class OPENPF2PLAYGROUND_API IInputBindableCharacterInterface
{
	GENERATED_BODY()

public:
	/**
	 * Loads input action bindings for all of the granted, activatable abilities of this character.
	 *
	 * If this character has any abilities already bound to input, those bindings are cleared before the input is bound.
	 * If the input bindings component is already wired-up to input for this character, the actions are bound to input
	 * actions immediately.
	 *
	 * On the client side, this should be invoked by the player controller whenever the abilities of this character may
	 * have changed. At a minimum, this must be invoked when a player is taking control of a character. This is a
	 * responsibility of the player controller taking ownership rather than the character being owned because it is only
	 * appropriate when a character is owned by a controller that processes input (i.e., it would not apply when a
	 * character is possessed/controlled by an AI controller).
	 */
	UFUNCTION(BlueprintCallable, Category="OpenPF2|Characters|Ability Bindings")
	virtual void LoadInputAbilityBindings() = 0;

	/**
	 * Configures this character to refresh action bindings whenever abilities in the character's ASC change locally.
	 *
	 * This has no effect in each of the following situations:
	 *   - If this is called on the server (since this is only relevant to clients).
	 *   - The character has no ASC.
	 *   - The character has already setup an ability change listener on this client.
	 */
	virtual void SetupClientAbilityChangeListener() = 0;
};
