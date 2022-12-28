// OpenPF2 for UE Game Logic, Copyright 2022, Guy Elsmore-Paddock. All Rights Reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
// distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include <Engine/AssetManager.h>

#include "OpenPF2PlaygroundAssetManager.generated.h"

/**
 * Custom asset manager that ensures ability system globals needed for target data replication are initialized properly.
 *
 * Adapted from:
 * - https://github.com/tranek/GASDocumentation/blob/master/Source/GASDocumentation/Public/GDAssetManager.h
 * - https://github.com/tranek/GASDocumentation/blob/master/Source/GASDocumentation/Private/GDAssetManager.cpp
 */
UCLASS()
// ReSharper disable once CppClassCanBeFinal
class OPENPF2PLAYGROUND_API UOpenPF2PlaygroundAssetManager : public UAssetManager
{
	GENERATED_BODY()

public:
	// =================================================================================================================
	// Public Methods - UAssetManager Overrides
	// =================================================================================================================
	virtual void StartInitialLoading() override;
};
