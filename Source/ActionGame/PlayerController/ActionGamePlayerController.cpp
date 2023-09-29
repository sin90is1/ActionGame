// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerController/ActionGamePlayerController.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "ActionGameGameMode.h"

/*on the callback of our timer we will restart the player
but before restarting the timer we are going to change our status to Spectator, this will reset the pawn for the player controller
in this method we want to spawn a new pawn and if you still want to have the character you should do it differently.
and our original character will still be laying down in the word*/
void AActionGamePlayerController::RestartPlayerIn(float InTime)
{
	ChangeState(NAME_Spectating);

	GetWorld()->GetTimerManager().SetTimer(RestartPlayerTimerHandle, this, &AActionGamePlayerController::RestartPlayer, InTime, false);
}

//we will subscribe to DeathStateTag, we use are AbilitySystemComponent that we get from BlueprintLibrary by passing it aPawn
void AActionGamePlayerController::OnPossess(APawn* aPawn)
{
	Super::OnPossess(aPawn);

	if (UAbilitySystemComponent* AbilityComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(aPawn))
	{
		DeathStateTagDelegate = AbilityComponent->RegisterGameplayTagEvent(FGameplayTag::RequestGameplayTag(TEXT("State.Dead")), EGameplayTagEventType::NewOrRemoved).AddUObject(this, &AActionGamePlayerController::OnPawnDeathStateChanged);
	}
}

//here we just want to Unsubscribe from death event.
//Just for some reason we want to UnPossess the pawn, but the Pawn is still alive and we haven't received the death event yet.
// Still, we might want to Unsubscribe in this case, so we're going to first check whether we have it's valid.And then we will actually Unsubscribe.
void AActionGamePlayerController::OnUnPossess()
{
	Super::OnUnPossess();

	if (DeathStateTagDelegate.IsValid())
	{
		if (UAbilitySystemComponent* AbilityComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetPawn()))
		{
			AbilityComponent->UnregisterGameplayTagEvent(DeathStateTagDelegate, FGameplayTag::RequestGameplayTag(TEXT("State.Dead")), EGameplayTagEventType::NewOrRemoved);
		}
	}
}

//Here we going to check the amount of the state. If it's greater than 0, it means something has applied the death state, so we consider it being dead.
//And here we are going to have to get our game mode and notify it about our Pawn being killed.
void AActionGamePlayerController::OnPawnDeathStateChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	if (NewCount > 0)
	{
		UWorld* World = GetWorld();

		AActionGameGameMode* GameMode = World ? Cast<AActionGameGameMode>(World->GetAuthGameMode()) : nullptr;

		if (GameMode)
		{
			GameMode->NotifyPlayerDied(this);
		}
		//Just for safety we too here Unsubscribe from death event
		if (DeathStateTagDelegate.IsValid())
		{
			if (UAbilitySystemComponent* AbilityComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetPawn()))
			{
				AbilityComponent->UnregisterGameplayTagEvent(DeathStateTagDelegate, FGameplayTag::RequestGameplayTag(TEXT("State.Dead")), EGameplayTagEventType::NewOrRemoved);
			}
		}
	}
}

//we get our GameMode an ask our GameMode that we want you to restart the player.
//GameMode by default has the functionality to do that using the player start. we can override it for our GameMode but the default one will be enough for us
void AActionGamePlayerController::RestartPlayer()
{
	UWorld* World = GetWorld();

	AActionGameGameMode* GameMode = World ? Cast<AActionGameGameMode>(World->GetAuthGameMode()) : nullptr;

	if (GameMode)
	{
		GameMode->RestartPlayer(this);
		UE_LOG(LogTemp, Warning, TEXT("NotifyPlayerDied::PlayerController is valid : %s"), *GameMode->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("AActionGamePlayerController::RestartPlayer GameMode is Null"));
	}
}
