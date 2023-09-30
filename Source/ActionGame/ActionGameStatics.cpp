// Fill out your copyright notice in the Description page of Project Settings.


#include "ActionGameStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "AbilitySystemComponent.h"
#include "Actors/Projectile.h"
#include "AbilitySystemBlueprintLibrary.h"

static TAutoConsoleVariable<int32> CVarShowRadialDamage(
	TEXT("ShowDebugRadialDamage"),
	0,
	TEXT("Draws debug info about radial damage")
	TEXT(" 0: 0ff/n")
	TEXT(" 1: On/n"),
	ECVF_Cheat
);

const UItemStaticData* UActionGameStatics::GetItemStaticData(TSubclassOf< UItemStaticData> ItemDataClass)
{
	if (ItemDataClass)
	{
		return GetDefault<UItemStaticData>(ItemDataClass);
	}

	return nullptr;
}

/*RadialDamage is gonna work like this. We're gonna do first the SphearTrace to just get all the actors in the available range.
And then we'll do the line trace to each of them to see whether it's reachual. And if it is, we applied the effect.*/
void UActionGameStatics::ApplyRadialDamage(UObject* WorldContextObject, class AActor* DamageCauser, FVector Location, float Radius,float DamageAmount,
	TArray<TSubclassOf<class UGameplayEffect>> DamageEffects, const TArray<TEnumAsByte<EObjectTypeQuery>>& ObjectTypes, ETraceTypeQuery TraceType)
{
	//First the OutActors This is going to be the list of all the actors that are in sphear radius We want to apply the radial damage.
	TArray<AActor*> OutActors;

	//Next, the list of actors we should ignore here we're going to just add the damage causer.
	//if we skip this our shooter can be able to be harmed buy it's rocket
	TArray<AActor*> ActorsToIgnore = { DamageCauser };

	UKismetSystemLibrary::SphereOverlapActors(WorldContextObject, Location, Radius, ObjectTypes, nullptr, ActorsToIgnore, OutActors);

	//cash our debug console variable here
	const bool bDebug = static_cast<bool>(CVarShowRadialDamage.GetValueOnAnyThread());

	//iterate over all actors we have detected with our SphereTrace
	for (AActor* Actor : OutActors)
	{
		//we need HitResulte to be returned from our LineTrace
		FHitResult HitResult;

		if (UKismetSystemLibrary::LineTraceSingle(WorldContextObject, Location, Actor->GetActorLocation(), TraceType, true, ActorsToIgnore, EDrawDebugTrace::None, HitResult, true))
		{
			AActor* Target = HitResult.GetActor();
			//we check if our HitResulte target is the Actor we are looking for
			if (Target == Actor)
			{
				bool bWasApplied = false;

				if (UAbilitySystemComponent* AbilityComponenet = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target))
				{
					FGameplayEffectContextHandle EffectContext = AbilityComponenet->MakeEffectContext();
					EffectContext.AddInstigator(DamageCauser, DamageCauser);

					/*And now we will iterate over our effects and will apply every one of them.*/
					for (auto Effect : DamageEffects)
					{
						/*We gonna need an EffectsSpecHandle and we can use that handle in the same way we previously did for our weapons.
						Because we will set the damage we passed to this ApplyRadialDamage function to our EffectsSpec, using this We will also 
						kind of allow using the data-driven approach. So we're going to kind of pass the value to this ApplyRadialDamage function, 
						but it will be applied via the GameplayEffects and we can use the same attribute Tag that we have created for weapons recently.*/
						FGameplayEffectSpecHandle SpecHandle = AbilityComponenet->MakeOutgoingSpec(Effect, 1, EffectContext);
						if (SpecHandle.IsValid())
						{
							/*And using this code we will insert the damage value passed to this function to the applied effect. 
							Of course it gonna work only if inside this effect we have the modifier attribute that expects these 
							valid beset we're the tag we specified. Otherwise it just won't do anything, but Effects still going to be applied.*/
							UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, FGameplayTag::RequestGameplayTag(TEXT("Attribute.Health")), -DamageAmount);
							//finally apply our EffectsSpec
							FActiveGameplayEffectHandle ActiveGEHandle = AbilityComponenet->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());

							if (ActiveGEHandle.WasSuccessfullyApplied())
							{
								bWasApplied = true;
							}
						}
					}
				}

				if (bDebug)
				{
					DrawDebugLine(WorldContextObject->GetWorld(), Location, Actor->GetActorLocation(), bWasApplied ? FColor::Green : FColor::Red, false, 4.f, 0, 1);
					DrawDebugSphere(WorldContextObject->GetWorld(), HitResult.Location, 16, 16, bWasApplied ? FColor::Green : FColor::Red, false, 4.f, 0, 1);
					DrawDebugString(WorldContextObject->GetWorld(), HitResult.Location, *GetNameSafe(Target), nullptr, FColor::White, 0, false, 1.f);
				}
			}
			else
			{
				if (bDebug)
				{
					DrawDebugLine(WorldContextObject->GetWorld(), Location, Actor->GetActorLocation(), FColor::Red, false, 4.f, 0, 1);
					DrawDebugSphere(WorldContextObject->GetWorld(), HitResult.Location, 16, 16, FColor::Red, false, 4.f, 0, 1);
					DrawDebugString(WorldContextObject->GetWorld(), HitResult.Location, *GetNameSafe(Target), nullptr, FColor::Red, 0, false, 1.f);
				}
			}
		}
		else
		{
			if (bDebug)
			{
				DrawDebugLine(WorldContextObject->GetWorld(), Location, Actor->GetActorLocation(), FColor::Red, false, 4.f, 0, 1);
				DrawDebugSphere(WorldContextObject->GetWorld(), HitResult.Location, 16, 16, FColor::Red, false, 4.f, 0, 1);
				DrawDebugString(WorldContextObject->GetWorld(), HitResult.Location, *GetNameSafe(HitResult.GetActor()), nullptr, FColor::Red, 0, false, 1.f);
			}
		}
	}

	if (bDebug)
	{
		DrawDebugSphere(WorldContextObject->GetWorld(), Location, Radius, 16, FColor::Red, false, 4.f, 0, 1);
	}
}

AProjectile* UActionGameStatics::LaunchProjectile(UObject* WorldContextObject, TSubclassOf<UProjectileStaticData> ProjectileDataClass, FTransform Transform, AActor* Owner, APawn* Instigator)
{
	UWorld* World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;

	/*We will check whether our world is valid and whether it's a server. 
	Because we have to spawn the replicated actors only on the server, 
	then they will be automatically replicated to all the clients.*/
	if (World && World->IsServer())
	{
		/*We're going to use a function SpawnActorDeferred to spawn an actor, 
		but then have some time to set its basic data and only after that finish spawning. 
		So all functions like the BeginPlaywill be called after that.*/
		if (AProjectile* Projectile = World->SpawnActorDeferred<AProjectile>(AProjectile::StaticClass(), Transform, Owner, Instigator, ESpawnActorCollisionHandlingMethod::AlwaysSpawn))
		{
			/*And technically the only variable we want to set, in this case it's the StaticDataClass. 
			So later when the BeginPlay will be called after we finished spawn. 
			It will correctly launch and initialize the visuals of our projectile.*/
			Projectile->ProjectileDataClass = ProjectileDataClass;
			Projectile->FinishSpawning(Transform);

			return Projectile;
		}
	}

	return nullptr;
}
