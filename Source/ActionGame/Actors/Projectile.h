// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ActionGameTypes.h"
#include "Projectile.generated.h"

UCLASS()
class ACTIONGAME_API AProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AProjectile();

	/*Because projectiles are going to be using the same approach with StaticData,
	it's better to have a function that will return us these data, because by default 
	we would store that data as a class, not as an object.*/
	const UProjectileStaticData* GetProjectileData() const;

	/*We need a replicated variable of projectileStaticClass itself*/
	UPROPERTY(BlueprintReadOnly, Replicated)
	TSubclassOf<UProjectileStaticData> ProjectileDataClass;


protected:

	/*What's really gonna make a Projectile a Projectile is a projectile movement component.*/
	UPROPERTY(EditDefaultsOnly)
		class UProjectileMovementComponent* ProjectileMovementComponent = nullptr;

	//for a simple visualization of the projectile
	UPROPERTY()
		class UStaticMeshComponent* StaticMeshComponent = nullptr;


protected:
	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/*to play OnStop Effect we are going to use OnEndPlay function, we're going to use this 
	one because it will be called both for the server and all the clients because all of them 
	might want to play the OnStop Effects but the projectile will be spawned on the 
	server and then going to be just replicated to all the clients.*/
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/*we are going to do this just for the debug while we can use the same method for 
	any kind of projectile movement predictions. like if you want to visualize the path of
	the grenade that you are going to throw, so the player is able to see how it'll fly and 
	where it will be fall*/
	void DebugDrawPath() const;

	/*There is already made functionality for us in the ProjectileMovementComponent 
	that helps us to determine when the projectile has stopped and there is a delegate
	we can subscribe to that will return us also the hit result and we can use those huge result.
	So now this is the callback function. And on this function we will actually explode and apply damage.*/
	UFUNCTION()
	void OnProjectileStop(const FHitResult& ImpactResult);


	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

};
