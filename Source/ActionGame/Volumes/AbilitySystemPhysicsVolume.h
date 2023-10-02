// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PhysicsVolume.h"
#include "GameplayAbilitySpec.h"
#include "GameplayTagContainer.h"
#include "AbilitySystemPhysicsVolume.generated.h"



class UGameplayEffect;
class UGameplayAbility;

/*because we have to track onGoinigEffects and OnGoinigAbilities for every actor that it is in the volume
it's better to create a struck to unite all the data for the actor while it is inside of the volume*/
USTRUCT(BlueprintType)
struct FAbilityVolumeEnteredActorInfo
{
	GENERATED_USTRUCT_BODY()

	TArray<FGameplayAbilitySpecHandle> AppliedAbilities;

	TArray<FActiveGameplayEffectHandle> AppliedEffects;
};

UCLASS()
class ACTIONGAME_API AAbilitySystemPhysicsVolume : public APhysicsVolume
{
	GENERATED_BODY()

protected:

	//this effects will be applied to the character when character stays inside of the volume
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<TSubclassOf<UGameplayEffect>> OngoingEffectsToApply;

	//the effects when the character leaves the volume
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<TSubclassOf<UGameplayEffect>> OnExitEffectsToApply;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bDrawDebug = false;

	//we would have an array of gameplay Tags that we are going to send as gameplay events to the
	//actors who come inside of the volum both at the moment that enters and when leaves and this is for 
	//when Enters
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FGameplayTag> GameplayEventsToSendOnEnter;

	//array of GamePlayTags when we leave the volume
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FGameplayTag> GameplayEventsToSendOnExit;

	//abilities to gift to player while is inside of the volume
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<TSubclassOf<UGameplayAbility>> OngoingAbilitiesToGive;

	//abilities to give to player permanently when comes into the volume
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<TSubclassOf<UGameplayAbility>> PermanentAbilitiesToGive;

	//a simple TMap to store alll the actors that we have inside of the volume and
	//coresponding struct with the applied handles data
	//there is a trade off between the runtime speed or the memory allocation when choosing between array or map
	TMap<AActor*, FAbilityVolumeEnteredActorInfo> EnteredActorsInfoMap;

public:

	//we need to enable the tick where we draw our debug shape
	AAbilitySystemPhysicsVolume();

	//callbacks functions from PhysicsVolume that can track Actors who Entered or leaves it
	virtual void ActorEnteredVolume(class AActor* Other) override;

	virtual void ActorLeavingVolume(class AActor* Other) override;

	virtual void Tick(float DeltaSeconds) override;
};

//note: gameplayEvents can be handled differently some specific systems like inventory can track some of them
//but also they can trigger some abilities on the character, so this opens up possibilities for creating something like
//JumpPads that when you enter it it forces you to jump