// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActionGameTypes.generated.h"

class AItemActor;
class UGameplayAbility;
class UGameplayEffect;
class UAnimMontage;
class UNiagaraSystem;

USTRUCT(BlueprintType)
struct FCharacterData
{
	GENERATED_USTRUCT_BODY();

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GAS")
	TArray<TSubclassOf<class UGameplayEffect>> Effects;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "GAS")
	TArray<TSubclassOf<class UGameplayAbility>> Abilities;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Animation")
	class UCharacterAnimDataAsset* CharacterAnimDataAsset;

};

USTRUCT(BlueprintType)
struct FCharacterAnimationData
{
	GENERATED_USTRUCT_BODY();

	UPROPERTY(EditDefaultsOnly)
	class UBlendSpace* MovementBlendSpace = nullptr;

	UPROPERTY(EditDefaultsOnly)
	class UAnimSequenceBase* IdleAnimationAsset = nullptr;


	UPROPERTY(EditDefaultsOnly)
	class UBlendSpace* CrouchMovementBlendSpace = nullptr;

	UPROPERTY(EditDefaultsOnly)
	class UAnimSequenceBase* CrouchIdleAnimationAsset = nullptr;
};

UENUM(BlueprintType)
enum class EFoot : uint8
{
	Left  UMETA(DisplayName = "Left"),
	Right UMETA(DisplayName = "Right")
};

UCLASS(BlueprintType, Blueprintable)
class UItemStaticData : public UObject
{

	GENERATED_BODY()

public:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName Name;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<AItemActor> ItemActorClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName AttachmentSocket = NAME_None;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	bool bCanBeEquipped = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FCharacterAnimationData CharacterAnimationData;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<TSubclassOf<UGameplayAbility>> GrantedAbilities;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<TSubclassOf<UGameplayEffect>> OnGoingEffects;
};

UCLASS(BlueprintType, Blueprintable)
class UWeaponStaticData : public UItemStaticData
{

	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UGameplayEffect> DamageEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	USkeletalMesh* SkeletatMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UStaticMesh* StaticMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UAnimMontage* AttackMontage; 

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float ShootingDistance;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float FireRate;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	float BaseDamage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	USoundBase* AttackSound;
};

//we are going to work with it same way we are going to work with InventoryItemClass
UCLASS(BlueprintType, Blueprintable)
class UProjectileStaticData : public UObject
{
	GENERATED_BODY()

public:
	//apply it in the same way we applied the weapon
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		float BaseDamage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		float DamageRadius;

	/*Projectile movement simulation uses gravity.That's why we're going to also have a property for the gravity multiplayer.
	For example, if you're going to use a rocket, you might want disabled gravity completely, so setting this to zero. 
	And if your project are going to be some sort of grenade, you might want to use gravity. So we will parameterize this.*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		float GravityMultiplier;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		float InitialSpeed = 3000.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		float MaxSpeed = 3000.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		UStaticMesh* StaticMesh;

	/*when projectile is going to hit something. We will have to apply the radial damage.
	For this purpose we will have the array of effects. We will apply these effects to 
	all the target we will be able to reach using our radial damage.*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		TArray<TSubclassOf<UGameplayEffect>> Effects;

	/*we need few parameters for Tracing, first one will be used for sphere trace to find
	all the potential targets to apply radial damage.*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		TArray<TEnumAsByte<EObjectTypeQuery>> RadialDamageQueryTypes;

	/*second one is gonna be for linear trace.Because for all the targets we cannot detect in the 
	sphere,	we will try to check by using the line trace whether they are reachable and if they 
	are we will try to apply to them are effects*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		TEnumAsByte<ETraceTypeQuery> RadialDamageTraceType;

	/*for visual effects on stop. We also will create the VFX using Niagara system*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		UNiagaraSystem* OnStopVFX = nullptr;

	/*the sound base to play some sound when the projectile explodes.*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
		USoundBase* OnStopSFX = nullptr;

};

UENUM(BlueprintType)
enum EItemState 
{
	None UMETA(Displayname = "None"),
	Equipped UMETA(Displayname = "Equipped"),
	Dropped UMETA(Displayname = "Dropped")
};

UENUM(BlueprintType)
enum class EMovementDirectionType : uint8
{
	None UMETA(Displayname = "None"),
	//our default option
	OrientToMovement UMETA(Displayname = "OrientToMovement"),
	//character is going to look at the same direction that we are looking
	Strafe UMETA(Displayname = "Strafe")
};

