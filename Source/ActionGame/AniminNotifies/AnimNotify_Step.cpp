// Fill out your copyright notice in the Description page of Project Settings.


#include "AniminNotifies/AnimNotify_Step.h"

#include "ActionGameCharacter.h"
#include "ActorComponents/FootstepsComponent.h"


void UAnimNotify_Step::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	check(MeshComp);

	AActionGameCharacter* Character = MeshComp ? Cast<AActionGameCharacter>(MeshComp->GetOwner()) : nullptr;
	if (Character)
	{
		if (UFootstepsComponent* FootstepsComponent = Character->GetFootstepsComponent())
		{
			FootstepsComponent->HandleFootstep(foot);
		}
	}
}
