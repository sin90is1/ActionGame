// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/ItemActors/WeaponItemActor.h"

#include "Inventory/InventoryItemInstance.h"
#include "ActionGameTypes.h"

#include "Kismet/GameplayStatics.h"
#include "PhysicalMaterial/AG_PhysicalMaterial.h"
#include "NiagaraFunctionLibrary.h"

AWeaponItemActor::AWeaponItemActor()
{

}

const UWeaponStaticData* AWeaponItemActor::GetWeaponStaticData() const
{
	return ItemInstance ? Cast<UWeaponStaticData>(ItemInstance->GetItemsStaticData()) : nullptr;
}

FVector AWeaponItemActor::GetMuzzleLocation() const
{
	return MeshComponent ? MeshComponent->GetSocketLocation(TEXT("Muzzle")) : GetActorLocation();
}

float AWeaponItemActor::GetShootingDistance() const
{
	return ItemInstance ? GetWeaponStaticData()->ShootingDistance : 0.0;
}


//we check which skeletal or static property is set to valid and based on that we create a component at runtime and will attach this component to our root component and it will represents our weapon
//USkeletalMeshComponent will be created in both server and client and replication on this stage is not needed
void AWeaponItemActor::InitInternal()
{
	Super::InitInternal();

	if (const UWeaponStaticData* WeaponData = GetWeaponStaticData())
	{
		if (WeaponData->SkeletatMesh)
		{
			USkeletalMeshComponent* SkeletalComp = NewObject<USkeletalMeshComponent>(this, USkeletalMeshComponent::StaticClass(), TEXT("MeshComponent"));
			if (SkeletalComp)
			{
				SkeletalComp->RegisterComponent();
				SkeletalComp->SetSkeletalMesh(WeaponData->SkeletatMesh);
				SkeletalComp->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);

				MeshComponent = SkeletalComp;
			}
		} else if (WeaponData->StaticMesh)
		{
			UStaticMeshComponent* StaticComp = NewObject<UStaticMeshComponent>(this, UStaticMeshComponent::StaticClass(), TEXT("MeshComponent"));
			if (StaticComp)
			{
				StaticComp->RegisterComponent();
				StaticComp->SetStaticMesh(WeaponData->StaticMesh);
				StaticComp->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::SnapToTargetNotIncludingScale);

				MeshComponent = StaticComp;
			}
		}
	}
}

void AWeaponItemActor::PlayWeaponEffects(const FHitResult& InHitResult)
{
	if (HasAuthority())
	{
		MulticastPlayWeaponEffects(InHitResult);
	}
	else
	{
		PlayWeaponEffectsInternal(InHitResult);
	}
}

void AWeaponItemActor::MulticastPlayWeaponEffects_Implementation(const FHitResult& InHitResult)
{
	if (!Owner || Owner->GetLocalRole() != ROLE_AutonomousProxy)
	{
		PlayWeaponEffectsInternal(InHitResult);
	}
}

void AWeaponItemActor::PlayWeaponEffectsInternal(const FHitResult& InHitResult)
{
	if (InHitResult.PhysMaterial.Get())
	{
		UAG_PhysicalMaterial* PhysicalMaterial = Cast<UAG_PhysicalMaterial>(InHitResult.PhysMaterial.Get());

		if (PhysicalMaterial)
		{
			UGameplayStatics::PlaySoundAtLocation(this, PhysicalMaterial->PointImpactSound, InHitResult.ImpactPoint);

			UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, PhysicalMaterial->PointImpactVFX, InHitResult.ImpactPoint);
		}
	}

	if (const UWeaponStaticData* WeaponData = GetWeaponStaticData())
	{
		UGameplayStatics::PlaySoundAtLocation(this, WeaponData->AttackSound, GetActorLocation());
	}
}