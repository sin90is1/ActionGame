// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Projectile.h"

#include "GameFramework/ProjectileMovementComponent.h"
#include "NiagaraFunctionLibrary.h"

#include "Kismet/GameplayStatics.h"
#include "ActionGameStatics.h"

#include "Net/UnrealNetwork.h"


static TAutoConsoleVariable<int32> CVarShowProjectiles(
	TEXT("ShowDebugProjectiles"),
	0,
	TEXT("Draws debug info about projectiles")
	TEXT("	0: off/n")
	TEXT("	1: on/n"),
	ECVF_Cheat
);

// Sets default values
AProjectile::AProjectile()
{
 	PrimaryActorTick.bCanEverTick = false;

	//because projectile is a moving replicated object
	SetReplicateMovement(true);
	bReplicates = true;

	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));

	//set a few starting parameters just to make sure nothing unexpectable happen
	//until we actually start the projectile with the correct movement data
	ProjectileMovementComponent->ProjectileGravityScale = 0;
	ProjectileMovementComponent->Velocity = FVector::ZeroVector;
	//here we subscribe to our OnStopDelegate
	ProjectileMovementComponent->OnProjectileStop.AddDynamic(this, &AProjectile::OnProjectileStop);


	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	StaticMeshComponent->SetupAttachment(GetRootComponent());
	StaticMeshComponent->SetIsReplicated(true);
	StaticMeshComponent->SetCollisionProfileName(TEXT("Projectile"));
	StaticMeshComponent->SetReceivesDecals(false);

}

const UProjectileStaticData* AProjectile::GetProjectileData() const
{
	if (IsValid(ProjectileDataClass)) 
	{
		return GetDefault<UProjectileStaticData>(ProjectileDataClass);
	}
		
	return nullptr;
}

// Called when the game starts or when spawned
void AProjectile::BeginPlay()
{
	Super::BeginPlay();

	const UProjectileStaticData* ProjectileData = GetProjectileData();

	if (ProjectileData && ProjectileMovementComponent) 
	{
		if (ProjectileData->StaticMesh) 
		{
			StaticMeshComponent->SetStaticMesh(ProjectileData->StaticMesh);
		}
		ProjectileMovementComponent->bInitialVelocityInLocalSpace = false;
		ProjectileMovementComponent->InitialSpeed = ProjectileData->InitialSpeed;
		ProjectileMovementComponent->MaxSpeed = ProjectileData->MaxSpeed;
		ProjectileMovementComponent->bRotationFollowsVelocity = true;
		ProjectileMovementComponent->bShouldBounce = false;
		ProjectileMovementComponent->Bounciness = 0.f;
		ProjectileMovementComponent->ProjectileGravityScale = ProjectileData->GravityMultiplier;

		//to make our projectile start flying we have to set our velocity 
		//also we assume the projectile have the correct transforms at the BeginPlay
		ProjectileMovementComponent->Velocity = ProjectileData->InitialSpeed * GetActorForwardVector();
	}

	const int32 DebugShowProjectile = CVarShowProjectiles.GetValueOnAnyThread();
	if (DebugShowProjectile > 0) 
	{
		DebugDrawPath();
	}
	
}

void AProjectile::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	const UProjectileStaticData* ProjectileData = GetProjectileData();

	if (ProjectileData)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ProjectileData->OnStopSFX, GetActorLocation(), 1.f);
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, ProjectileData->OnStopVFX, GetActorLocation());
	}

	Super::EndPlay(EndPlayReason);
}

void AProjectile::DebugDrawPath() const
{
	const UProjectileStaticData* ProjectileData = GetProjectileData();

	if (ProjectileData) 
	{
		FPredictProjectilePathParams PredictionProjectileParams;

		PredictionProjectileParams.StartLocation = GetActorLocation();
		PredictionProjectileParams.LaunchVelocity = ProjectileData->InitialSpeed * GetActorForwardVector();
		PredictionProjectileParams.TraceChannel = ECollisionChannel::ECC_Visibility;
		PredictionProjectileParams.bTraceComplex = true;
		PredictionProjectileParams.bTraceWithCollision = true;
		PredictionProjectileParams.DrawDebugType = EDrawDebugTrace::ForDuration;
		PredictionProjectileParams.DrawDebugTime = 3.f;
		PredictionProjectileParams.OverrideGravityZ = ProjectileData->GravityMultiplier == 0 ? 0.0001f : ProjectileData->GravityMultiplier;

		

		FPredictProjectilePathResult PredictProjectilePathResult;

		if (UGameplayStatics::PredictProjectilePath(this, PredictionProjectileParams, PredictProjectilePathResult)) 
		{
			DrawDebugSphere(GetWorld(), PredictProjectilePathResult.HitResult.Location, 50, 10, FColor::Red);
		}
	}
}

void AProjectile::OnProjectileStop(const FHitResult& ImpactResult)
{
	const UProjectileStaticData* ProjectileData = GetProjectileData();

	if (ProjectileData) 
	{
		//We do not spawn our effects here because we would rely on collision to Author and the server have to 
		//do this by destroying the projectile but then EndPlay would be replicated and we play it for everyone
		UActionGameStatics::ApplyRadialDamage(this, GetOwner(), GetActorLocation(),
			ProjectileData->DamageRadius,
			ProjectileData->BaseDamage,
			ProjectileData->Effects,
			ProjectileData->RadialDamageQueryTypes,
			ProjectileData->RadialDamageTraceType
		);
	}
	
	Destroy();
}

void AProjectile::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AProjectile, ProjectileDataClass);
}
