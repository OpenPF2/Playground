// OpenPF2 for UE Game Logic, Copyright 2022, Guy Elsmore-Paddock. All Rights Reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
// distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.

#pragma once

#include "PF2PlayerControllerBase.h"
#include "OpenPF2PlaygroundPlayerControllerBase.generated.h"

/**
 * Base class for OpenPF2 Playground player controllers.
 */
UCLASS()
class OPENPF2PLAYGROUND_API AOpenPF2PlaygroundPlayerControllerBase : public APF2PlayerControllerBase
{
	GENERATED_BODY()

protected:
	/**
	 * An input component for directing player actions to whichever character is currently being controlled.
	 */
	UPROPERTY(BlueprintReadOnly)
	UInputComponent* ControlledCharacterInputComponent;

	// =================================================================================================================
	// Protected Methods - APlayerController Overrides
	// =================================================================================================================
	virtual void AcknowledgePossession(APawn* InPawn) override;
	virtual void SetupInputComponent() override;
};
