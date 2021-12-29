// OpenPF2 for UE Game Logic, Copyright 2021, Guy Elsmore-Paddock. All Rights Reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
// distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include <CoreMinimal.h>

#include "OpenPF2PlaygroundDowntimeModeRuleSet.h"
#include "OpenPF2PlaygroundEncounterModeRuleSet.h"
#include "OpenPF2PlaygroundExplorationModeRuleSet.h"
#include "OpenPF2PlaygroundNullModeRuleSet.h"

#include "GameModes/PF2GameModeBase.h"

#include "OpenPF2PlaygroundGameMode.generated.h"

/**
 * Default game mode for the OpenPF2 Playground sample.
 */
UCLASS(MinimalAPI)
// ReSharper disable once CppClassCanBeFinal
class AOpenPF2PlaygroundGameMode : public APF2GameModeBase
{
	GENERATED_BODY()

protected:
	// =================================================================================================================
	// Protected Constants
	// =================================================================================================================
	/**
	 * Static map of modes of play to rule sets.
	 */
	const TMap<EPF2ModeOfPlayType, UClass *> ModeRuleSets =
	{
		{ EPF2ModeOfPlayType::None,        UOpenPF2PlaygroundNullModeRuleSet::StaticClass()        },
		{ EPF2ModeOfPlayType::Exploration, UOpenPF2PlaygroundExplorationModeRuleSet::StaticClass() },
		{ EPF2ModeOfPlayType::Encounter,   UOpenPF2PlaygroundEncounterModeRuleSet::StaticClass()   },
		{ EPF2ModeOfPlayType::Downtime,    UOpenPF2PlaygroundDowntimeModeRuleSet::StaticClass()    },
	};

public:
	// =================================================================================================================
	// Public Constructors
	// =================================================================================================================
	/**
	 * Default constructor for AOpenPF2PlaygroundGameMode.
	 */
	explicit AOpenPF2PlaygroundGameMode();

	// =================================================================================================================
	// Public Methods - IPF2GameModeInterface Implementation
	// =================================================================================================================
	virtual FORCEINLINE TScriptInterface<IPF2ModeOfPlayRuleSet> CreateModeOfPlayRuleSet(
		const EPF2ModeOfPlayType ModeOfPlay) override
	{
		const UClass* const RuleSetType = ModeRuleSets[ModeOfPlay];
		UObject*            NewRuleSet  = NewObject<UObject>(this, RuleSetType);

		return TScriptInterface<IPF2ModeOfPlayRuleSet>(NewRuleSet);
	}

	// =================================================================================================================
	// Public Methods - APF2GameModeBase Overrides
	// =================================================================================================================
	virtual void StartEncounterMode() override;
	virtual void StartExplorationMode() override;
	virtual void StartDowntimeMode() override;
};
