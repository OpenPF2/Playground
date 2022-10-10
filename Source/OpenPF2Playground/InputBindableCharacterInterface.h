// OpenPF2 for UE Game Logic, Copyright 2022, Guy Elsmore-Paddock. All Rights Reserved.
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
	 * have changed. At a minimum, this must be invoked when a character is being possessed by a player controller.
	 * This is a responsibility of the player controller rather than the character itself because it is only appropriate
	 * when a character is being possessed by a controller that processes input (i.e., it would not apply when a
	 * character is possessed by an AI controller).
	 */
	virtual void LoadInputActionBindings() = 0;
};
