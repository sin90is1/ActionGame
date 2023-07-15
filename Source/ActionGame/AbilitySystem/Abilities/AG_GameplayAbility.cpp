// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/Abilities/AG_GameplayAbility.h"

#include "AbilitySystemComponent.h"
#include "ActionGameCharacter.h"

void UAG_GameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	FGameplayEffectContextHandle EffectContext = ActorInfo->AbilitySystemComponent->MakeEffectContext();

	for (auto GameplayEffect : OnGoingEffectsToJustApplyOnStart)
	{
		if (!GameplayEffect.Get()) continue;

		if (UAbilitySystemComponent* AbilityComponent = ActorInfo->AbilitySystemComponent.Get())
		{
			FGameplayEffectSpecHandle SpecHandle = AbilityComponent->MakeOutgoingSpec(GameplayEffect, 1, EffectContext);
			if (SpecHandle.IsValid())
			{
				FActiveGameplayEffectHandle ActiveGEHandle = AbilityComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
				if (!ActiveGEHandle.WasSuccessfullyApplied())
				{
					//ABILITY_LOG(Log, TEXT("Ability %s Failed to apply startup effect %s"), *GetName(), *GetNameSafe(GameplayEffect));
				}
			}
		}
	}
	if (IsInstantiated())
	{
		for (auto GameplayEffect : OnGoingEffectsToRemoveOnEnd)
		{
			if (!GameplayEffect.Get()) continue;

			if (UAbilitySystemComponent* AbilityComponent = ActorInfo->AbilitySystemComponent.Get())
			{
				FGameplayEffectSpecHandle SpecHandle = AbilityComponent->MakeOutgoingSpec(GameplayEffect, 1, EffectContext);
				if (SpecHandle.IsValid())
				{
					FActiveGameplayEffectHandle ActiveGEHandle = AbilityComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
					if (!ActiveGEHandle.WasSuccessfullyApplied())
					{
						//ABILITY_LOG(Log, TEXT("Ability %s Failed to apply runtime effect %s"), *GetName(), *GetNameSafe(GameplayEffect));
					}
					else
					{
						RemoveOnEndEffectHandle.Add(ActiveGEHandle);
					}
				}
			
			}
		}
	}
}

void UAG_GameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (IsInstantiated())
	{
		for (FActiveGameplayEffectHandle ActiveEffectHandle : RemoveOnEndEffectHandle)
		{
			if (ActiveEffectHandle.IsValid())
			{
				ActorInfo->AbilitySystemComponent->RemoveActiveGameplayEffect(ActiveEffectHandle);
			}
		}

		RemoveOnEndEffectHandle.Empty();
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

AActionGameCharacter* UAG_GameplayAbility::GetActionGameCharacterFromActorInfo() const
{
	return Cast<AActionGameCharacter>(GetAvatarActorFromActorInfo());
}
