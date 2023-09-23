// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/ItemActor.h"
#include "WeaponItemActor.generated.h"

/**
 * 
 */
UCLASS()
class ACTIONGAME_API AWeaponItemActor : public AItemActor
{
	GENERATED_BODY()

public:

	AWeaponItemActor();

	const UWeaponStaticData* GetWeaponSataticData() const;

protected:

	//both skeletal mesh and static mesh are derived from MeshComponent so we can use just a MeshComponent variable to  store a pointer to both skeletal mesh and static mesh 
	UPROPERTY()
	UMeshComponent* MeshComponent = nullptr;

	//override init internal where we are going to initialize the visuals
	virtual void InitInternal() override;
};
