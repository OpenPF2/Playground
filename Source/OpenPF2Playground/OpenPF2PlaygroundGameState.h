// OpenPF2 for UE Game Logic, Copyright 2021-2023, Guy Elsmore-Paddock. All Rights Reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
// distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include "PF2GameStateBase.h"

#include "OpenPF2PlaygroundGameState.generated.h"

/**
 * Default game state for the OpenPF2 Playground sample.
 */
UCLASS(MinimalAPI)
// ReSharper disable once CppClassCanBeFinal
class AOpenPF2PlaygroundGameState : public APF2GameStateBase
{
	GENERATED_BODY()

protected:
	// =================================================================================================================
	// Protected Fields - Blueprint Accessible
	// =================================================================================================================
	/**
	 * How many enemies remain in the current encounter.
	 */
	UPROPERTY(Replicated)
	int32 RemainingEnemies;
};
