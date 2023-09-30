// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ActionGameTypes.h"
#include "ActionGameStatics.generated.h"

/**
 * 
 */
UCLASS()
class ACTIONGAME_API UActionGameStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintCallable, BlueprintPure)
	static const UItemStaticData* GetItemStaticData(TSubclassOf< UItemStaticData> ItemDataClass);

	/*This function is for applying radial damage. We can now create it as a global function in our BlueprintLibrary, 
	so any entity that might want to apply radial damage will be able to use it, not only projectiles.*/
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
	static void ApplyRadialDamage(UObject* WorldContextObject, class AActor* DamageCauser, FVector Location, float Radius, float DamageAmount, 
		TArray<TSubclassOf<class UGameplayEffect>> DamageEffects, const TArray<TEnumAsByte<EObjectTypeQuery>>& ObjectTypes, ETraceTypeQuery TraceType);

	/*This function is for exactly launching the projectile. Also we will implement it at the global function, 
	so again any entity will be able to launch the projectile from the class if it needs to.*/
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
	static AProjectile* LaunchProjectile(UObject* WorldContextObject, TSubclassOf<UProjectileStaticData> ProjectileDataClass, FTransform Transform, AActor* Owner, APawn* Instigator);

};
