// OpenPF2 for UE Game Logic, Copyright 2022-2023, Guy Elsmore-Paddock. All Rights Reserved.
//
// This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0. If a copy of the MPL was not
// distributed with this file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include "OpenPF2PlaygroundPlayerControllerBase.h"

#include "InputBindableCharacterInterface.h"
#include "PF2CharacterBase.h"
#include "PF2CharacterInterface.h"

AOpenPF2PlaygroundPlayerControllerBase::AOpenPF2PlaygroundPlayerControllerBase()
{
	// set our turn rates for input
	this->BaseTurnRate   = 45.0f;
	this->BaseLookUpRate = 45.0f;
}

void AOpenPF2PlaygroundPlayerControllerBase::SetPawn(APawn* InPawn)
{
	APF2CharacterBase*       OldCharacterIntf = Cast<APF2CharacterBase>(this->GetPawn());
	const APF2CharacterBase* NewCharacterIntf = Cast<APF2CharacterBase>(InPawn);

	Super::SetPawn(InPawn);

	if ((OldCharacterIntf != nullptr) && (OldCharacterIntf != NewCharacterIntf))
	{
		const UWorld*  World        = this->GetWorld();
		FTimerManager& TimerManager = World->GetTimerManager();

		// BUGBUG (UE-78453): There is a delay in replicating controller ownership changes to clients. So, if we refresh
		// the ASC during the same frame as the ownership change happens, the client will tend to cache the old
		// controller as the current controller of the old character even though it's not. This breaks execution of
		// abilities and montages, so as a workaround we schedule the ASC to be updated during the next frame after
		// replication has happened.
		TimerManager.SetTimerForNextTick(OldCharacterIntf, &APF2CharacterBase::InitializeOrRefreshAbilities);
	}
}

void AOpenPF2PlaygroundPlayerControllerBase::Native_OnCharacterGiven(
	const TScriptInterface<IPF2CharacterQueueInterface>& CharacterQueueComponent,
	const TScriptInterface<IPF2CharacterInterface>& GivenCharacter)
{
	Super::Native_OnCharacterGiven(CharacterQueueComponent, GivenCharacter);

	this->AcknowledgeOwnership(GivenCharacter);
}

void AOpenPF2PlaygroundPlayerControllerBase::AcknowledgeOwnership(
	const TScriptInterface<IPF2CharacterInterface> InCharacter) const
{
	IInputBindableCharacterInterface* BindableCharacterIntf =
		Cast<IInputBindableCharacterInterface>(InCharacter.GetObject());

	// Ensure the ASC has been initialized on the server.
	InCharacter->InitializeOrRefreshAbilities();

	if (BindableCharacterIntf != nullptr)
	{
		BindableCharacterIntf->LoadInputAbilityBindings();
		BindableCharacterIntf->SetupClientAbilityChangeListener();
	}
}

bool AOpenPF2PlaygroundPlayerControllerBase::GetHitResultForScreenPosition(const FVector2D         InPosition,
                                                                           const ECollisionChannel InTraceChannel,
                                                                           const bool              bInTraceComplex,
                                                                           FHitResult&             OutHitResult) const
{
	const bool bHit = this->GetHitResultAtScreenPosition(InPosition, InTraceChannel, bInTraceComplex, OutHitResult);

	if (!bHit)
	{
		// If there was no hit we reset the results. This is redundant but helps Blueprint users
		OutHitResult = FHitResult();
	}

	return bHit;
}

FVector2D AOpenPF2PlaygroundPlayerControllerBase::GetCenterOfViewport() const
{
	FVector2D                  CenterPosition;
	const ULocalPlayer*        LocalPlayer    = this->GetLocalPlayer();
	const UGameViewportClient* ViewportClient = LocalPlayer->ViewportClient;

	if ((LocalPlayer != nullptr) && (ViewportClient != nullptr))
	{
		FVector2D ViewportSize;

		ViewportClient->GetViewportSize(ViewportSize);

		CenterPosition = FVector2D(
			ViewportSize.X / 2.0f,
			ViewportSize.Y / 2.0f
		);
	}

	return CenterPosition;
}

void AOpenPF2PlaygroundPlayerControllerBase::Multicast_DisableAutomaticCameraManagement_Implementation()
{
	this->bAutoManageActiveCameraTarget = false;
}

void AOpenPF2PlaygroundPlayerControllerBase::Multicast_EnableAutomaticCameraManagement_Implementation()
{
	this->bAutoManageActiveCameraTarget = true;
}
