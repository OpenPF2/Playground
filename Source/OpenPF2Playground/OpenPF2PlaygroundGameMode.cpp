// OpenPF2 for UE Game Logic, Copyright 2021, Guy Elsmore-Paddock. All Rights Reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
// distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include "OpenPF2PlaygroundGameMode.h"

#include <UObject/ConstructorHelpers.h>

#include "OpenPF2PlaygroundCharacterBase.h"
#include "OpenPF2PlaygroundGameState.h"

AOpenPF2PlaygroundGameMode::AOpenPF2PlaygroundGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(
		TEXT("/Game/OpenPF2Playground/Characters/TwinBlast/BP_TwinBlastPF2Character")
	);

	static ConstructorHelpers::FClassFinder<APlayerController> PlayerControllerBPClass(
		TEXT("/Game/OpenPF2Playground/Characters/BP_PlayerController")
	);

	if (PlayerPawnBPClass.Class != nullptr)
	{
		this->DefaultPawnClass = PlayerPawnBPClass.Class;
	}

	if (PlayerControllerBPClass.Class != nullptr)
	{
		this->PlayerControllerClass = PlayerControllerBPClass.Class;
	}

	this->GameStateClass = AOpenPF2PlaygroundGameState::StaticClass();
}

void AOpenPF2PlaygroundGameMode::StartEncounterMode()
{
	Super::StartEncounterMode();
}

void AOpenPF2PlaygroundGameMode::StartExplorationMode()
{
	Super::StartExplorationMode();
}

void AOpenPF2PlaygroundGameMode::StartDowntimeMode()
{
	Super::StartDowntimeMode();
}
