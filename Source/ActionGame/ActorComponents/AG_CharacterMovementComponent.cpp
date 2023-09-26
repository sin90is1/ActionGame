// Fill out your copyright notice in the Description page of Project Settings.


#include "ActorComponents/AG_CharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"

static TAutoConsoleVariable<int32> CVarShowTraversal(
	TEXT("Show Debug Traversal"),
	0,
	TEXT("Draw debug info about traversal")
	TEXT("	0: off/n")
	TEXT("	1: on/n"),
	ECVF_Cheat);

bool UAG_CharacterMovementComponent::TryTraversal(UAbilitySystemComponent* ASC)
{
	for (TSubclassOf<UGameplayAbility> AbilityClass : TraversalAbilitiesOrdered)
	{
		if (ASC->TryActivateAbilityByClass(AbilityClass, true))
		{
			FGameplayAbilitySpec* Spec;

			Spec = ASC->FindAbilitySpecFromClass(AbilityClass);
			if (Spec && Spec->IsActive())
			{
				return true;
			}
		}
	}

	return false;
}

void UAG_CharacterMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	HandleMovementDirection();

	if (UAbilitySystemComponent* AbilityComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwner()))
	{
		AbilityComponent->RegisterGameplayTagEvent(FGameplayTag::RequestGameplayTag(TEXT("Movement.Enforced.Strafe"), EGameplayTagEventType::NewOrRemoved)).AddUObject(this, &UAG_CharacterMovementComponent::OnEnforcedStrafeTagChanged);
	}
}

EMovementDirectionType UAG_CharacterMovementComponent::GetMovementDirectionType() const
{
	return MovementDirectionType;
}

void UAG_CharacterMovementComponent::SetMovementDirectionType(EMovementDirectionType InMovementDirectionType)
{
	MovementDirectionType = InMovementDirectionType;

	HandleMovementDirection();
}

void UAG_CharacterMovementComponent::OnEnforcedStrafeTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	if (NewCount)
	{
		SetMovementDirectionType(EMovementDirectionType::Strafe);
	}
	else
	{
		SetMovementDirectionType(EMovementDirectionType::OrientToMovement);
	}
}

void UAG_CharacterMovementComponent::HandleMovementDirection()
{
	switch (MovementDirectionType)
	{
	case EMovementDirectionType::Strafe:
		bUseControllerDesiredRotation = true;
		bOrientRotationToMovement = false;
		CharacterOwner->bUseControllerRotationYaw = true;
		break;
	default:
		bUseControllerDesiredRotation = false;
		bOrientRotationToMovement = true;
		CharacterOwner->bUseControllerRotationYaw = false;
		break;
	}
}
