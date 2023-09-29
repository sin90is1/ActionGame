// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/GA_InventoryAbility.h"
#include "GA_InventoryCombatAbility.generated.h"

/**
 * 
 */
UCLASS()
class ACTIONGAME_API UGA_InventoryCombatAbility : public UGA_InventoryAbility
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintPure)
	FGameplayEffectSpecHandle GetWeaponEffectSpec(const FHitResult& InHitResult);

	//aim with 2 line trace
	// first one is from the camera and show where we are looking at
	// second one is from the weapon to that focus
	// this function tell us whether we have a blocking hit at the final line trace
	UFUNCTION(BlueprintPure)
	const bool GetWeaponToFocusTraceResult(float TraceDistance, ETraceTypeQuery TraceType, FHitResult& OutHitResult);
};
