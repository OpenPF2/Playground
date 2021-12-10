// Copyright 2021 Guy Elsmore-Paddock. All Rights Reserved.
// Adapted from content that is Copyright Epic Games, Inc. (Third Person Sample).
// Licensed only for use with Unreal Engine.

#include "OpenPF2PlaygroundGameMode.h"

#include <UObject/ConstructorHelpers.h>

#include "OpenPF2PlaygroundCharacterBase.h"

AOpenPF2PlaygroundGameMode::AOpenPF2PlaygroundGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(
		TEXT("/Game/OpenPF2Playground/Characters/TwinBlast/BP_TwinBlastPF2Character")
	);

	if (PlayerPawnBPClass.Class != nullptr)
	{
		this->DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
