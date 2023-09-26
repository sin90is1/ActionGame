// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ActionGameTypes.h"
#include "GameplayAbilitySpec.h"
#include "InventoryItemInstance.generated.h"

/**
 * 
 */
class AItemActor;

UCLASS(BlueprintType, Blueprintable)
class ACTIONGAME_API UInventoryItemInstance : public UObject
{
	GENERATED_BODY()

public:

	virtual void Init(TSubclassOf<UItemStaticData> InItemStaticDataClass);

	virtual bool IsSupportedForNetworking() const override { return true; }
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	const UItemStaticData* GetItemsStaticData() const;

	UPROPERTY(Replicated)
	TSubclassOf<UItemStaticData> ItemStaticDataClass;

	UPROPERTY(ReplicatedUsing = OnRep_Equipped)
	bool bEquipped = false; 

	UFUNCTION()
	void OnRep_Equipped();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void OnEquipped(AActor* InOwner = nullptr);
	//because we want to grant and remove abilities we need to have access to our AbilitySystemComponent and we can do that buy having the owner (AActor* InOwner = nullptr)
	virtual void OnUnequipped(AActor* InOwner = nullptr);
	virtual void OnDropped(AActor* InOwner = nullptr);

	UFUNCTION(BlueprintPure)
	AItemActor* GetItemActor() const;

protected:
	
	UPROPERTY(Replicated)
	AItemActor* ItemActor = nullptr;

	void TryGrantAbilities(AActor* InOwner);

	void TryRemoveAbilities(AActor* InOwner);

	void TryApplyEffects(AActor* InOwner);

	void TryRemoveEffects(AActor* InOwner);

	UPROPERTY()
	TArray<FGameplayAbilitySpecHandle> GrantedAbilityHandles;

	TArray<FActiveGameplayEffectHandle> OnGoingEffectHandles;
};
