// OpenPF2 for UE Game Logic, Copyright 2022, Guy Elsmore-Paddock. All Rights Reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
// distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include "OpenPF2PlaygroundPlayerControllerBase.h"

void AOpenPF2PlaygroundPlayerControllerBase::SetupInputComponent()
{
	Super::SetupInputComponent();

	this->ControlledCharacterInputComponent =
		NewObject<UInputComponent>(this, UInputComponent::StaticClass(), TEXT("PC_ControlledCharacterInput"));

	this->ControlledCharacterInputComponent->RegisterComponent();

	this->PushInputComponent(this->ControlledCharacterInputComponent);
}
