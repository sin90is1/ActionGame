// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/InventoryItemInstance.h"
#include "ActionGameStatics.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"

#include "Actors/ItemActor.h"

void UInventoryItemInstance::init(TSubclassOf<UItemStaticData> InItemStaticDataClass)
{
	ItemStaticDataClass = InItemStaticDataClass;
}

const UItemStaticData* UInventoryItemInstance::GetItemsStaticData() const
{
	return UActionGameStatics::GetItemStaticData(ItemStaticDataClass);
}


void UInventoryItemInstance::OnRep_Equipped()
{

}

void UInventoryItemInstance::OnEquipped(AActor* InOwner)
{
	//UE_LOG(LogTemp, Warning, TEXT("UInventoryItemInstance::OnEquipped  world is %s"), IsValid(world) ? TEXT("valid") : TEXT("invalid"));
	//UE_LOG(LogTemp, Warning, TEXT("UInventoryItemInstance::OnEquipped  InOwner is %s"), IsValid(InOwner) ? TEXT("valid") : TEXT("invalid"));
	if (UWorld* World = InOwner->GetWorld())
	{
		const UItemStaticData* StaticData = GetItemsStaticData();
		FTransform Transform;
		ItemActor = World->SpawnActorDeferred<AItemActor>(StaticData->ItemActorClass, Transform, InOwner);
		ItemActor->Init(this);
		ItemActor->OnEquipped();
		ItemActor->FinishSpawning(Transform);

		ACharacter* Character = Cast<ACharacter>(InOwner);
		if (USkeletalMeshComponent* SkeletalMesh = Character ? Character->GetMesh() : nullptr)
		{
			ItemActor->AttachToComponent(SkeletalMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, StaticData->AttachmentSocket);
		}
	}

		/*    	ACharacter* Character = Cast<ACharacter>(InOwner);
		USkeletalMeshComponent* SkeletalMesh = Character ? Character->GetMesh() : nullptr;

		if (SkeletalMesh)
		{
			FAttachmentTransformRules AttachRules(EAttachmentRule::SnapToTarget, // Location rule
				EAttachmentRule::SnapToTarget, // Rotation rule
				EAttachmentRule::KeepRelative, // Scale rule - Keep relative transform including scale
				false);
			ItemActor->AttachToComponent(SkeletalMesh, AttachRules, StaticData->AttachmentSocket);
			UE_LOG(LogTemp, Warning, TEXT("UInventoryItemInstance::OnEquipped // attachment succeded"));
		}
	}*/
	//maybe put it in above condition
	bEquipped = true;
}

void UInventoryItemInstance::OnUnequipped()
{
	if (ItemActor)
	{
		ItemActor->Destroy();
		ItemActor = nullptr;
	}

	bEquipped = false;
}

void UInventoryItemInstance::OnDropped()
{
	if (ItemActor)
	{
		ItemActor->OnDropped();
	}
	bEquipped = false;
}

void UInventoryItemInstance::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UInventoryItemInstance, ItemStaticDataClass);
	DOREPLIFETIME(UInventoryItemInstance, bEquipped);
	DOREPLIFETIME(UInventoryItemInstance, ItemActor);
}
