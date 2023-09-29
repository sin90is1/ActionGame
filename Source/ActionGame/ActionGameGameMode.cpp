// Copyright Epic Games, Inc. All Rights Reserved.

#include "ActionGameGameMode.h"
#include "ActionGameCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "PlayerController/ActionGamePlayerController.h"

AActionGameGameMode::AActionGameGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
	PlayerControllerClass = AActionGamePlayerController::StaticClass();
}

//PlayerController will pass itself to this function
void AActionGameGameMode::NotifyPlayerDied(AActionGamePlayerController* PlayerController)
{
	if (PlayerController) 
	{
		PlayerController->RestartPlayerIn(2.f);
		UE_LOG(LogTemp, Warning, TEXT("NotifyPlayerDied::PlayerController is valid : %s"), *PlayerController->GetName());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("NotifyPlayerDied::PlayerController is NUll"));
	}
}
