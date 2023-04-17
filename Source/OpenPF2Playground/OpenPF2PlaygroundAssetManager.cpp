// OpenPF2 for UE Game Logic, Copyright 2022, Guy Elsmore-Paddock. All Rights Reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
// distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include "OpenPF2PlaygroundAssetManager.h"

#include <AbilitySystemGlobals.h>

void UOpenPF2PlaygroundAssetManager::StartInitialLoading()
{
	Super::StartInitialLoading();

	// This fixes the following errors when attempting to use target data:
	// - LogAbilitySystem: Could not find GameplayAbilityTargetData_SingleTargetHit in ScriptStructCache.
	// - LogAbilitySystem: Could not script struct at idx 59
	// - ReceivedBunch: Invalid replicated field 0 in PF2CharacterCommand
	// - LogNet: UActorChannel::ProcessBunch: Replicator.ReceivedBunch failed.  Closing connection.
	// - LogNet: UEngine::BroadcastNetworkFailure: FailureType = ConnectionLost, ErrorString = Your connection to the host has been lost.
	UAbilitySystemGlobals::Get().InitGlobalData();
}
