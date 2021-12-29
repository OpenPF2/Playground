// OpenPF2 for UE Game Logic, Copyright 2021, Guy Elsmore-Paddock. All Rights Reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
// distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once
#include "GameModes/PF2ModeOfPlayRuleSet.h"

#include "OpenPF2PlaygroundExplorationModeRuleSet.generated.h"

UCLASS(BlueprintType, Blueprintable)
class UOpenPF2PlaygroundExplorationModeRuleSet final : public UObject, public IPF2ModeOfPlayRuleSet
{
	GENERATED_BODY()

public:
	virtual bool CanTransitionTo(const IPF2GameStateInterface* PF2GameState, EPF2ModeOfPlayType TargetMode) const override
	{
		// Can always transition out of exploration.
		return true;
	};
};
