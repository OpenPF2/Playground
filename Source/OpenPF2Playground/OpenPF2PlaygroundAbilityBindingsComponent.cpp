// OpenPF2 for UE Game Logic, Copyright 2023, Guy Elsmore-Paddock. All Rights Reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
// distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include "OpenPF2PlaygroundAbilityBindingsComponent.h"

#include <GameFramework/GameStateBase.h>

#include "PF2GameStateInterface.h"

#include "Libraries/PF2AbilitySystemLibrary.h"

FGameplayEventData UOpenPF2PlaygroundAbilityBindingsComponent::BuildPayloadForAbilityActivation(
	const FGameplayAbilitySpecHandle AbilitySpecHandle)
{
	AGameStateBase*               GameState     = this->GetWorld()->GetGameState();
	const IPF2GameStateInterface* GameStateIntf = Cast<IPF2GameStateInterface>(GameState);

	FGameplayEventData Result = UPF2AbilityBindingsComponent::BuildPayloadForAbilityActivation(AbilitySpecHandle);

	// Only capture movement grid targets when in encounter mode. In other modes, abilities get executed immediately.
	if ((GameStateIntf != nullptr) && (GameStateIntf->GetModeOfPlay() == EPF2ModeOfPlayType::Encounter))
	{
		const IPF2CharacterInterface* CharacterIntf = this->GetOwningCharacter();

		if (CharacterIntf != nullptr)
		{
			const TScriptInterface<IPF2PlayerControllerInterface> PlayerController =
				CharacterIntf->GetPlayerController();

			if (PlayerController == nullptr)
			{
				UE_LOG(
					LogPf2CoreAbilities,
					Error,
					TEXT("Player controller is null or not compatible with OpenPF2, so ability target cannot be captured.")
				);
			}
			else
			{
				// Add any current target from a movement grid to the ability activation.
				Result.TargetData =
					UPF2AbilitySystemLibrary::CreateAbilityTargetDataFromPlayerControllerTargetSelection(PlayerController);

				// Clear selection for next ability activation.
				PlayerController->ClearTargetSelection();
			}
		}
	}

	return Result;
}
