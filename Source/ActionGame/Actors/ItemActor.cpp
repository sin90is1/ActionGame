// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/ItemActor.h"
#include "Engine/ActorChannel.h"
#include "Net/UnrealNetwork.h"
#include "Components/SphereComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Inventory/InventoryItemInstance.h"
#include "ActorComponents/InventoryComponent.h"


// Sets default values
AItemActor::AItemActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(true);

	SphereComponent2 = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	SphereComponent2->SetupAttachment(RootComponent);
	SphereComponent2->OnComponentBeginOverlap.AddDynamic(this, &AItemActor::OnSphereOverlap);

	// In the constructor
	UE_LOG(LogTemp, Warning, TEXT("SphereComponent: %s"), *SphereComponent2->GetName());
}


void AItemActor::Init(UInventoryItemInstance* InInstance)
{
	ItemInstance = InInstance;

	InitInternal();
}

// Called when the game starts or when spawned
// void AItemActor::BeginPlay()
// {
// 	Super::BeginPlay();
// 
// 	if (HasAuthority())
// 	{
// 		if (!IsValid(ItemInstance) && IsValid(ItemStaticDataClass))
// 		{
// 			ItemInstance = NewObject<UInventoryItemInstance>();
// 			ItemInstance->init(ItemStaticDataClass);
// 
// 			SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
// 			SphereComponent->SetGenerateOverlapEvents(true);
// 
// 			InitInternal();
// 		}
// 	}
// }
void AItemActor::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		if (!IsValid(ItemInstance) && IsValid(ItemStaticDataClass))
		{
			ItemInstance = NewObject<UInventoryItemInstance>();
			ItemInstance->Init(ItemStaticDataClass);
			if (IsValid(SphereComponent2))
			{
				SphereComponent2->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
				SphereComponent2->SetGenerateOverlapEvents(true);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("BeginPlay - SphereComponen is NULL"));
			}


			InitInternal();
		}
	}
}
void AItemActor::OnEquipped()
{
	ItemState = EItemState::Equipped;


	if (IsValid(SphereComponent2))
	{
		SphereComponent2->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		SphereComponent2->SetGenerateOverlapEvents(false);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("OnEquipped - SphereComponen is NULL"));
	}
}

void AItemActor::OnUnEquipped()
{
	ItemState = EItemState::None;

	if (IsValid(SphereComponent2))
	{
		SphereComponent2->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		SphereComponent2->SetGenerateOverlapEvents(false);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("OnUnEquipped - SphereComponen is NULL"));
	}
}

void AItemActor::OnDropped()
{

	ItemState = EItemState::Dropped;

	GetRootComponent()->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);

	if (AActor* ActorOwner = GetOwner()) 
	{
		const FVector Location = GetActorLocation();
		const FVector ForwardVector = ActorOwner->GetActorForwardVector();

		const float dropItemDist = 100.f;
		const float dropItemTraceDist = 100.f;

		const FVector TraceStart = Location + ForwardVector * dropItemDist;
		const FVector TraceEnd = TraceStart - FVector::UpVector * dropItemTraceDist;

		TArray<AActor*> ActorsToIgnore = { GetOwner() };

		FHitResult TraceHit;

		static const auto CVar = IConsoleManager::Get().FindConsoleVariable(TEXT("ShowDebugInventory"));
		const bool bShowInventoryDebug = CVar->GetInt() > 0;

		FVector TargetLocation = TraceEnd;

		EDrawDebugTrace::Type DebugDrawType = bShowInventoryDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None;
		if (UKismetSystemLibrary::LineTraceSingleByProfile(this, TraceStart, TraceEnd, TEXT("worldStatic"), true, ActorsToIgnore, DebugDrawType, TraceHit, true)) 
		{
			if (TraceHit.bBlockingHit) 
			{
				TargetLocation = TraceHit.Location;
			}
		}
		SetActorLocation(TargetLocation);


		if (IsValid(SphereComponent2))
		{
			SphereComponent2->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
			SphereComponent2->SetGenerateOverlapEvents(true);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("OnDropped - SphereComponen is NULL"));
		}
	}
}

// void AItemActor::OnDropped()
// {
// 	ItemState = EItemState::Dropped;
// 
// 	GetRootComponent()->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
// 
// 	if (AActor* ActorOwner = GetOwner())
// 	{
// 		const FVector Location = GetActorLocation();
// 		const FVector Forward = ActorOwner->GetActorForwardVector();
// 
// 		const float DropItemDist = 100.f;
// 		const float DropItemTraceDist = 1000.f;
// 
// 		const FVector TraceStart = Location + Forward * DropItemDist;
// 		const FVector TraceEnd = TraceStart - FVector::UpVector * DropItemTraceDist;
// 
// 		TArray<AActor*> ActorsToIgnore = { GetOwner() };
// 
// 		FHitResult TraceHit;
// 
// 		static const auto CVar = IConsoleManager::Get().FindConsoleVariable(TEXT("ShowDebugInventory"));
// 		const bool bShowInventory = CVar->GetInt() > 0;
// 
// 		FVector TargetLocation = TraceEnd;
// 
// 		EDrawDebugTrace::Type DebugDrawType = bShowInventory ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None;
// 		if (UKismetSystemLibrary::LineTraceSingleByProfile(this, TraceStart, TraceEnd, TEXT("WorldStatic"), true, ActorsToIgnore, DebugDrawType, TraceHit, true))
// 		{
// 			if (TraceHit.bBlockingHit)
// 			{
// 				TargetLocation = TraceHit.Location;
// 			}
// 		}
// 		SetActorLocation(TargetLocation);
// 	}
// 
// 	SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
// 	SphereComponent->SetGenerateOverlapEvents(true);
// }

bool AItemActor::ReplicateSubobjects(class UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool WroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

	WroteSomething |= Channel->ReplicateSubobject(ItemInstance, *Bunch, *RepFlags);

	return WroteSomething;
}


void AItemActor::OnRep_ItemInstance(UInventoryItemInstance* OldItemInstance)
{
	if (IsValid(ItemInstance) && !IsValid(OldItemInstance))
	{
		InitInternal();
	}
}

void AItemActor::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SpeepResult)
{
	if (HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("OnSphereOverlapL"));
		FGameplayEventData EventPayload;
		EventPayload.Instigator = this;
		EventPayload.OptionalObject = ItemInstance;
		EventPayload.EventTag = UInventoryComponent::EquipItemActorTag;

		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(OtherActor, UInventoryComponent::EquipItemActorTag, EventPayload);
	}
}

void AItemActor::OnRep_ItemState()
{
	if (IsValid(SphereComponent2))
	{
		switch (ItemState)
		{
		case EItemState::Equipped:
			SphereComponent2->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			SphereComponent2->SetGenerateOverlapEvents(false);
			break;

		case EItemState::None:
			SphereComponent2->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			SphereComponent2->SetGenerateOverlapEvents(false);
			break;

		case EItemState::Dropped:
			SphereComponent2->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
			SphereComponent2->SetGenerateOverlapEvents(true);
			break;

		default:
			SphereComponent2->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
			SphereComponent2->SetGenerateOverlapEvents(true);
			break;
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("OnRep_ItemState - SphereComponen is NULL"));
	}
	
}

void AItemActor::InitInternal()
{
	//overrided in child class
}

// Called every frame
void AItemActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AItemActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AItemActor, ItemInstance);
	DOREPLIFETIME(AItemActor, ItemState);
}
