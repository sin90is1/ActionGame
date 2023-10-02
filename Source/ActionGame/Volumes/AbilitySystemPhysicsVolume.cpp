// Fill out your copyright notice in the Description page of Project Settings.


#include "Volumes/AbilitySystemPhysicsVolume.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "DrawDebugHelpers.h"

AAbilitySystemPhysicsVolume::AAbilitySystemPhysicsVolume()
{
	//in order to use tick we have enable it
	PrimaryActorTick.bCanEverTick = true;
}

void AAbilitySystemPhysicsVolume::ActorEnteredVolume(class AActor* Other)
{
	Super::ActorEnteredVolume(Other);

	//if we don't have an Authority we don't do anything
	if (!HasAuthority()) return;

	if (UAbilitySystemComponent* AbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Other))
	{
		//iterate over our PermanentAbilitiesToGive and give it to our entered actors
		for (auto Ability : PermanentAbilitiesToGive)
		{
			AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(Ability));
		}

		//add the actor to our map
		EnteredActorsInfoMap.Add(Other);

		for (auto Ability : OngoingAbilitiesToGive)
		{
			FGameplayAbilitySpecHandle AbilityHandle = AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(Ability));

			//we get the ability handle and add it to our map
			EnteredActorsInfoMap[Other].AppliedAbilities.Add(AbilityHandle);
		}

		for (auto GameplayEffect : OngoingEffectsToApply)
		{
			//as usual we get the EffectContext
			FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();

			//we add information to our EffectContext, like as an instigator or sourceObject or whatevere you going to
			//use on the other end and in our case we use addInstigator
			EffectContext.AddInstigator(Other, Other);

			FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(GameplayEffect, 1, EffectContext);

			if (SpecHandle.IsValid())
			{
				FActiveGameplayEffectHandle ActiveGEHandle = AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());

				if (ActiveGEHandle.WasSuccessfullyApplied())
				{
					//if it was successfullyApplied we add it to tour Msp for the current actor
					EnteredActorsInfoMap[Other].AppliedEffects.Add(ActiveGEHandle);
				}
			}
		}

		//the gameplay events that we have to send on entered
		for (auto EventTag : GameplayEventsToSendOnEnter)
		{
			//as usual we create the event Payload
			FGameplayEventData EventPayload;
			//set the Tag
			EventPayload.EventTag = EventTag;
			//send the event to our actor
			UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Other, EventTag, EventPayload);
		}
	}
}

//apply OnExitEvent and rollback the effects and the abilities we have given to the actor
void AAbilitySystemPhysicsVolume::ActorLeavingVolume(class AActor* Other)
{
	Super::ActorLeavingVolume(Other);

	if (!HasAuthority()) return;

	if (UAbilitySystemComponent* AbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Other))
	{
		//we check to see if we have the record for this actor
		if (EnteredActorsInfoMap.Find(Other))
		{
			//we use the map to access the actors data and we iterate over AppliedEffects and AbilityHandles to remove them
			for (auto GameplayEffectHandle : EnteredActorsInfoMap[Other].AppliedEffects)
			{
				AbilitySystemComponent->RemoveActiveGameplayEffect(GameplayEffectHandle);
			}

			for (auto GameplayAbilityHandle : EnteredActorsInfoMap[Other].AppliedAbilities)
			{
				AbilitySystemComponent->ClearAbility(GameplayAbilityHandle);
			}

			//we remove this actor from our map
			EnteredActorsInfoMap.Remove(Other);
		}


		for (auto GameplayEffect : OnExitEffectsToApply)
		{
			FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();

			EffectContext.AddInstigator(Other, Other);

			FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(GameplayEffect, 1, EffectContext);

			if (SpecHandle.IsValid())
			{
				FActiveGameplayEffectHandle ActiveGEHandle = AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
			}
		}

		//apply the effects for when the actor leaves the volume
		for (auto EventTag : GameplayEventsToSendOnExit)
		{
			FGameplayEventData EventPayload;
			EventPayload.EventTag = EventTag;

			UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Other, EventTag, EventPayload);
		}
	}
}

void AAbilitySystemPhysicsVolume::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	//draw a box that represent our volume
	if (bDrawDebug)
	{
		DrawDebugBox(GetWorld(), GetActorLocation(), GetBounds().BoxExtent, FColor::Red, false, 0, 0, 5);
	}
}
