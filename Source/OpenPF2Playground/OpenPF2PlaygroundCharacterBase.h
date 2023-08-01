// Copyright 2021-2023 Guy Elsmore-Paddock. All Rights Reserved.
// Adapted from content that is Copyright Epic Games, Inc. (Third Person Sample).
// Licensed only for use with Unreal Engine.

#pragma once

#include <Camera/CameraComponent.h>

#include <GameFramework/SpringArmComponent.h>

#include "InputBindableCharacterInterface.h"
#include "PF2CharacterBase.h"
#include "OpenPF2PlaygroundCharacterBase.generated.h"

// =====================================================================================================================
// Forward Declarations (to minimize header dependencies)
// =====================================================================================================================
class UPF2AbilityBindingsComponent;

// =====================================================================================================================
// Normal Declarations
// =====================================================================================================================
/**
 * Base class for all characters in the OpenPF2 Playground sample.
 */
UCLASS(Config=Game)
// ReSharper disable once CppClassCanBeFinal
class AOpenPF2PlaygroundCharacterBase : public APF2CharacterBase, public IInputBindableCharacterInterface
{
	GENERATED_BODY()

protected:
	/**
	 * Camera boom positioning the camera behind the character.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	USpringArmComponent* CameraBoom;

	/**
	 * Camera that follows the player in third-person when outside of encounters.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	UCameraComponent* FollowCamera;

	/**
	 * Component that enables character abilities to be bound to input in a dynamic/configurable way at run-time.
	 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	UPF2AbilityBindingsComponent* AbilityBindings;

public:
	/**
	 * Default constructor.
	 */
	explicit AOpenPF2PlaygroundCharacterBase();

	// =================================================================================================================
	// Public Methods - IInputBindableCharacterInterface Implementation
	// =================================================================================================================
	virtual void LoadInputAbilityBindings() override;

	virtual void SetupClientAbilityChangeListener() override;

	virtual TScriptInterface<IPF2AbilityBindingsInterface> GetAbilityBindingsComponent() const override;

	// =================================================================================================================
	// Public Methods
	// =================================================================================================================
	/**
	 * Gets the camera boom sub-object.
	 *
	 * @return
	 *	The camera boom.
	 */
	FORCEINLINE USpringArmComponent* GetCameraBoom() const
	{
		return this->CameraBoom;
	}

	/**
	 * Gets the "follow camera" sub-object.
	 *
	 * This is a camera that provides the player with a view behind the third-person character.
	 *
	 * @return
	 *	The camera.
	 */
	FORCEINLINE UCameraComponent* GetFollowCamera() const
	{
		return this->FollowCamera;
	}

protected:
	// =================================================================================================================
	// Protected Methods - APawn Overrides
	// =================================================================================================================
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	// =================================================================================================================
	// Protected Native Event Callbacks
	// =================================================================================================================
	/**
	 * Native event fired when the character's abilities have finished being replicated from the server.
	 *
	 * @param Asc
	 *	The ability system component that has notified this character abilities are loaded.
	 */
	UFUNCTION()
	virtual void Native_OnAbilitiesLoaded(const TScriptInterface<IPF2AbilitySystemInterface>& Asc);
};
