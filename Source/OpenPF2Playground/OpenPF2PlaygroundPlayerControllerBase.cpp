// OpenPF2 for UE Game Logic, Copyright 2022, Guy Elsmore-Paddock. All Rights Reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
// distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include "OpenPF2PlaygroundPlayerControllerBase.h"

#include "InputBindableCharacterInterface.h"
#include "PF2CharacterInterface.h"

void AOpenPF2PlaygroundPlayerControllerBase::OnPossess(APawn* InPawn)
{
	IPF2CharacterInterface* CharacterIntf = Cast<IPF2CharacterInterface>(InPawn);

	Super::OnPossess(InPawn);

	if (CharacterIntf != nullptr)
	{
		CharacterIntf->InitializeAbilities();
	}
}

void AOpenPF2PlaygroundPlayerControllerBase::AcknowledgePossession(APawn* NewPawn)
{
	IPF2CharacterInterface*           CharacterIntf         = Cast<IPF2CharacterInterface>(NewPawn);
	IInputBindableCharacterInterface* BindableCharacterIntf = Cast<IInputBindableCharacterInterface>(NewPawn);

	Super::AcknowledgePossession(NewPawn);

	if (CharacterIntf != nullptr)
	{
		CharacterIntf->InitializeAbilities();
	}

	if (BindableCharacterIntf != nullptr)
	{
		BindableCharacterIntf->LoadInputActionBindings();
	}
}

void AOpenPF2PlaygroundPlayerControllerBase::SetupInputComponent()
{
	Super::SetupInputComponent();

	this->ControlledCharacterInputComponent =
		NewObject<UInputComponent>(this, UInputComponent::StaticClass(), TEXT("PC_ControlledCharacterInput"));

	this->ControlledCharacterInputComponent->RegisterComponent();

	this->PushInputComponent(this->ControlledCharacterInputComponent);
}
