// Copyright 2021 Guy Elsmore-Paddock. All Rights Reserved.
// Adapted from content that is Copyright Epic Games, Inc. (Third Person Sample).
// Licensed only for use with Unreal Engine.

#pragma once

#include <CoreMinimal.h>
#include <GameFramework/GameModeBase.h>

#include "OpenPF2PlaygroundGameMode.generated.h"

/**
 * Default game mode for the OpenPF2 Playground sample.
 */
UCLASS(MinimalAPI)
// ReSharper disable once CppClassCanBeFinal
class AOpenPF2PlaygroundGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AOpenPF2PlaygroundGameMode();
};
