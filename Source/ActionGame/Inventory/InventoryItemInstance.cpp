// Fill out your copyright notice in the Description page of Project Settings.


#include "Inventory/InventoryItemInstance.h"
#include "ActionGameStatics.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Actors/ItemActor.h"

void UInventoryItemInstance::Init(TSubclassOf<UItemStaticData> InItemStaticDataClass)
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

// void UInventoryItemInstance::OnEquipped(AActor* InOwner)
// {
// 	if (UWorld* World = InOwner->GetWorld())
// 	{
// 		const UItemStaticData* StaticData = GetItemsStaticData();
// 		FTransform Transform;
// 		ItemActor = World->SpawnActorDeferred<AItemActor>(StaticData->ItemActorClass, Transform, InOwner);
// 		ItemActor->Init(this);
// 		ItemActor->OnEquipped();
// 		ItemActor->FinishSpawning(Transform);
// 
// 		ACharacter* Character = Cast<ACharacter>(InOwner);
// 		if (USkeletalMeshComponent* SkeletalMesh = Character ? Character->GetMesh() : nullptr)
// 		{
// 			ItemActor->AttachToComponent(SkeletalMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, StaticData->AttachmentSocket);
// 		}
// 	}
// 	/*    	ACharacter* Character = Cast<ACharacter>(InOwner);
// 		USkeletalMeshComponent* SkeletalMesh = Character ? Character->GetMesh() : nullptr;
// 
// 		if (SkeletalMesh)
// 		{
// 			FAttachmentTransformRules AttachRules(EAttachmentRule::SnapToTarget, // Location rule
// 				EAttachmentRule::SnapToTarget, // Rotation rule
// 				EAttachmentRule::KeepRelative, // Scale rule - Keep relative transform including scale
// 				false);
// 			ItemActor->AttachToComponent(SkeletalMesh, AttachRules, StaticData->AttachmentSocket);
// 			UE_LOG(LogTemp, Warning, TEXT("UInventoryItemInstance::OnEquipped // attachment succeded"));
// 		}
// 	}*/
// 	//maybe put it in above condition
// 	TryGrantAbilities(InOwner);
// 	
// 	bEquipped = true;
// }
void UInventoryItemInstance::OnEquipped(AActor* InOwner)
{
	if (UWorld* World = InOwner->GetWorld())
	{
		const UItemStaticData* StaticData = GetItemsStaticData();
		FTransform Transform;
		ItemActor = World->SpawnActorDeferred<AItemActor>(StaticData->ItemActorClass, Transform, InOwner);
		ItemActor->Init(this);
		ItemActor->OnEquipped();
		ItemActor->FinishSpawning(Transform);

		//UE_LOG(LogTemp, Warning, TEXT("UInventoryItemInstance::OnEquipped : ItemActor = %s "), ItemActor->ItemInstance->ItemStaticDataClass->Name.ToString())

		ACharacter* Character = Cast<ACharacter>(InOwner);
		if (USkeletalMeshComponent* SkeletalMesh = Character ? Character->GetMesh() : nullptr)
		{
			ItemActor->AttachToComponent(SkeletalMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, StaticData->AttachmentSocket);
		}
	}
	TryGrantAbilities(InOwner);
	bEquipped = true;
}

void UInventoryItemInstance::OnUnequipped(AActor* InOwner)
{
	if (ItemActor)
	{
		ItemActor->Destroy();
		ItemActor = nullptr;
	}

	TryRemoveAbilities(InOwner);
	bEquipped = false;
}

void UInventoryItemInstance::OnDropped(AActor* InOwner)
{
	if (ItemActor)
	{
		ItemActor->OnDropped();
	}
	TryRemoveAbilities(InOwner);
	bEquipped = false;
}

void UInventoryItemInstance::TryGrantAbilities(AActor* InOwner)
{
	if (InOwner && InOwner->HasAuthority())
	{
		if (UAbilitySystemComponent* AbilityComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(InOwner))
		{
			const UItemStaticData* StaticData = GetItemsStaticData();

			for (auto ItemAbility : StaticData->GrantedAbilities)
			{
				GrantedAbilityHandles.Add(AbilityComponent->GiveAbility(FGameplayAbilitySpec(ItemAbility)));
			}
		}
	}
}

void UInventoryItemInstance::TryRemoveAbilities(AActor* InOwner)
{
	if (InOwner && InOwner->HasAuthority())
	{
		if (UAbilitySystemComponent* AbilityComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(InOwner))
		{
			const UItemStaticData* StaticData = GetItemsStaticData();

			for (auto AbilityHandle : GrantedAbilityHandles)
			{
				AbilityComponent->ClearAbility(AbilityHandle);
			}
			//clear cashed abilities handles array
			GrantedAbilityHandles.Empty();
		}
	}
}

void UInventoryItemInstance::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UInventoryItemInstance, ItemStaticDataClass);
	DOREPLIFETIME(UInventoryItemInstance, bEquipped);
	DOREPLIFETIME(UInventoryItemInstance, ItemActor);
}
