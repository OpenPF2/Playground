// OpenPF2 for UE Game Logic, Copyright 2021, Guy Elsmore-Paddock. All Rights Reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
// distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include <CoreMinimal.h>

#include "PF2GameModeBase.h"

#include "OpenPF2PlaygroundGameMode.generated.h"

/**
 * Default game mode for the OpenPF2 Playground sample.
 */
UCLASS(MinimalAPI)
// ReSharper disable once CppClassCanBeFinal
class AOpenPF2PlaygroundGameMode : public APF2GameModeBase
{
	GENERATED_BODY()

public:
	// =================================================================================================================
	// Public Constructors
	// =================================================================================================================
	/**
	 * Default constructor for AOpenPF2PlaygroundGameMode.
	 */
	explicit AOpenPF2PlaygroundGameMode();

	// =================================================================================================================
	// Public Methods - APF2GameModeBase Overrides
	// =================================================================================================================
	virtual void StartEncounterMode() override;
	virtual void StartExplorationMode() override;
	virtual void StartDowntimeMode() override;
};
