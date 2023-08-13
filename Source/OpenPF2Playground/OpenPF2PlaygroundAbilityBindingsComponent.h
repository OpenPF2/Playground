// OpenPF2 for UE Game Logic, Copyright 2023, Guy Elsmore-Paddock. All Rights Reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
// distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include "Commands/PF2AbilityBindingsComponent.h"

#include "OpenPF2PlaygroundAbilityBindingsComponent.generated.h"

// =====================================================================================================================
// Normal Declarations
// =====================================================================================================================
/**
 * A specialized version of the ability bindings component that injects the current target into ability activations.
 */
UCLASS(ClassGroup="OpenPF2-Characters", meta=(BlueprintSpawnableComponent))
// ReSharper disable once CppClassCanBeFinal
class UOpenPF2PlaygroundAbilityBindingsComponent : public UPF2AbilityBindingsComponent
{
	GENERATED_BODY()

public:
	// =================================================================================================================
	// Public Constructor
	// =================================================================================================================
	/**
	 * Default constructor.
	 */
	explicit UOpenPF2PlaygroundAbilityBindingsComponent() : UPF2AbilityBindingsComponent()
	{
	}

	// =================================================================================================================
	// UPF2AbilityBindingsComponent Overrides
	// =================================================================================================================
	virtual FGameplayEventData BuildPayloadForAbilityActivation(
		const FGameplayAbilitySpecHandle AbilitySpecHandle) override;
};
