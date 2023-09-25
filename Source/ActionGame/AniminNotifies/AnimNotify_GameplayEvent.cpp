// Fill out your copyright notice in the Description page of Project Settings.


#include "AniminNotifies/AnimNotify_GameplayEvent.h"

#include "AbilitySystemBlueprintLibrary.h"

void UAnimNotify_GameplayEvent::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	check(MeshComp);

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(MeshComp->GetOwner(), Payload.EventTag, Payload);
}
