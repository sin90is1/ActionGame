// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GameplayTagContainer.h"
#include "ActionGamePlayerController.generated.h"

/**
 we use an ability to actually kill the character. so the ability will apply an status effect of "Death status"
 player controller will subscribe to the status change of the pawn it poses and so to our character and when it happens it will notify the GameMode

GameMode is a logical class which is derived from actor that exists only on the server and it serves as a some set of rules for our game.
we're going to do this this way exactly.Because different game modes may want to handle player death in a different way. 
They may want to ReSpawn the player, or they may want to maybe.Make him a spectator until the game and so this should be driven by the game rules, not by anything else.
That's why  PlayerController will be only some sort of notifier executor in this case, but not the one who will decide.

 But here is you can see the functionality we're going to do will allow us to track the death of the character.
 And also there is a function "RestartPlayerIn" this function will be called in our case by the GameMode because that's what our basic GameMode wants to do.
 After declare is killed, we will ask PlayerController to ReSpawn the character in a given time.
 */

UCLASS()
class ACTIONGAME_API AActionGamePlayerController : public APlayerController
{
	GENERATED_BODY()

public:

	void RestartPlayerIn(float InTime);

protected:

	virtual void OnPossess(APawn* aPawn) override;

	virtual void OnUnPossess() override;

	UFUNCTION()
	void OnPawnDeathStateChanged(const FGameplayTag CallbackTag, int32 NewCount);

	void RestartPlayer();

	FTimerHandle RestartPlayerTimerHandle;

	FDelegateHandle DeathStateTagDelegate;

};
