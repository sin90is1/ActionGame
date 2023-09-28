// Copyright Epic Games, Inc. All Rights Reserved.

#include "ActionGameCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

#include "AbilitySystemComponent.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystem/AttributeSets/AG_AttributeSetBase.h"
#include "DataAssets/CharacterDataAsset.h"
#include "AbilitySystem/Component/AG_AbilitySystemComponent.h"

#include "Net/UnrealNetwork.h"

#include "ActorComponents/AG_CharacterMovementComponent.h"
#include "ActorComponents/FootstepsComponent.h"
#include "ActorComponents/InventoryComponent.h"

//////////////////////////////////////////////////////////////////////////
// AActionGameCharacter


AActionGameCharacter::AActionGameCharacter(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer.SetDefaultSubobjectClass<UAG_CharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

	AGCharacterMovementComponent = Cast<UAG_CharacterMovementComponent>(GetCharacterMovement());

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)

	//Ability System
	AbilitySystemComponent = CreateDefaultSubobject<UAG_AbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	AttributeSet = CreateDefaultSubobject<UAG_AttributeSetBase>(TEXT("AttributeSet"));

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetMaxMovementSpeedAttribute()).AddUObject(this, &AActionGameCharacter::OnMaxMovementSpeedChanged);

	FootstepsComponent = CreateDefaultSubobject<UFootstepsComponent>(TEXT("FootstepsComponent"));

	AGMotionWarpingComponent = CreateDefaultSubobject<UAG_MotionWarpingComponent>(TEXT("MotionWarpingComponent"));

	InventoryComponent = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));
	InventoryComponent->SetIsReplicated(true);
}

void AActionGameCharacter::PostLoad()
{
	Super::PostLoad();

	if (IsValid(CharacterDataAsset))
	{
		SetCharacaterData(CharacterDataAsset->CharacterData);
	}
}

void AActionGameCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
// Input

void AActionGameCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent)) {

		//Moving
		if (MoveAction)
		{
			EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AActionGameCharacter::Move);
		}

		//Looking
		if (LookAction)
		{
			EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AActionGameCharacter::Look);
		}

		//Jumping
		if (JumpAction)
		{
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AActionGameCharacter::OnJumpAction);
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		}

		//Crouching
		if (CrouchAction)
		{
			EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &AActionGameCharacter::OnCrouchActionStarted);
			EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Completed, this, &AActionGameCharacter::OnCrouchActionEnded);
		}

		//Sprinting
		if (SprintAction)
		{
			EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &AActionGameCharacter::OnSprintActionStarted);
			EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &AActionGameCharacter::OnSprintActionEnded);
		}

		//DroppingItem
		if (DropItemAction)
		{
			EnhancedInputComponent->BindAction(DropItemAction, ETriggerEvent::Started, this, &AActionGameCharacter::OnDropItemAction);
		}

		//EquipNext
		if (EquipNextAction)
		{
			EnhancedInputComponent->BindAction(EquipNextAction, ETriggerEvent::Started, this, &AActionGameCharacter::OnEquipNextAction);
		}

		//Unequipping
		if (UnequipAction)
		{
			EnhancedInputComponent->BindAction(UnequipAction, ETriggerEvent::Started, this, &AActionGameCharacter::OnUnequipAction);
		}

		//Attacking
		if (AttackAction)
		{
			EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Started, this, &AActionGameCharacter::OnAttackActionStarted);
			EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Completed, this, &AActionGameCharacter::OnAttackActionEnded);
		}
	}

}

void AActionGameCharacter::OnJumpAction()
{
	AGCharacterMovementComponent->TryTraversal(AbilitySystemComponent);
}


void AActionGameCharacter::OnCrouchActionStarted(const FInputActionValue& Value)
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->TryActivateAbilitiesByTag(CrouchTags, true);
	}
}

void AActionGameCharacter::OnCrouchActionEnded(const FInputActionValue& Value)
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->CancelAbilities(&CrouchTags);
	}
}

void AActionGameCharacter::OnSprintActionStarted(const FInputActionValue& Value)
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->TryActivateAbilitiesByTag(SprintTags, true);
	}
}

void AActionGameCharacter::OnSprintActionEnded(const FInputActionValue& Value)
{
	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->CancelAbilities(&SprintTags);
	}
}

void AActionGameCharacter::OnDropItemAction(const FInputActionValue& Value)
{
	FGameplayEventData EventPayload;
	EventPayload.EventTag = UInventoryComponent::DropItemTag;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, UInventoryComponent::DropItemTag, EventPayload);
}

void AActionGameCharacter::OnEquipNextAction(const FInputActionValue& Value)
{
	FGameplayEventData EventPayload;
	EventPayload.EventTag = UInventoryComponent::EquipNextTag;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, UInventoryComponent::EquipNextTag, EventPayload);
}

void AActionGameCharacter::OnUnequipAction(const FInputActionValue& Value)
{
	FGameplayEventData EventPayload;
	EventPayload.EventTag = UInventoryComponent::UnequipTag;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, UInventoryComponent::UnequipTag, EventPayload);
}

void AActionGameCharacter::OnAttackActionStarted(const FInputActionValue& Value)
{
	FGameplayEventData EventPayload;
	EventPayload.EventTag = AttackStartedEventTag;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, AttackStartedEventTag, EventPayload);
}

void AActionGameCharacter::OnAttackActionEnded(const FInputActionValue& Value)
{
	FGameplayEventData EventPayload;
	EventPayload.EventTag = AttackEndedEventTag;

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(this, AttackEndedEventTag, EventPayload);
}

void AActionGameCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->RemoveActiveEffectsWithTags(InAirTags);
	}
}

void AActionGameCharacter::OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);

	if (!CrouchStateEffect.Get()) return;

	if (AbilitySystemComponent)
	{
		FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();

		FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(CrouchStateEffect, 1, EffectContext);
		if (SpecHandle.IsValid())
		{
			FActiveGameplayEffectHandle ActiveGEHandle = AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
			if (!ActiveGEHandle.WasSuccessfullyApplied())
			{
				//ABILITY_LOG(Log, TEXT("Ability %s Failed to apply Crouch effect %s"), *GetName(), *GetNameSafe(CrouchStateEffect));
			}
		}
	}
}

void AActionGameCharacter::OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);

	if (AbilitySystemComponent && CrouchStateEffect.Get())
	{
		AbilitySystemComponent->RemoveActiveGameplayEffectBySourceEffect(CrouchStateEffect, AbilitySystemComponent);
	}
}

UAbilitySystemComponent* AActionGameCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}


bool AActionGameCharacter::ApplyGameplayEffectToSelf(TSubclassOf<UGameplayEffect> Effect, FGameplayEffectContextHandle InEffectContext)
{
	if (!Effect.Get())
	return false;

	FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(Effect, 1, InEffectContext);
	if (SpecHandle.IsValid())
	{
		FActiveGameplayEffectHandle ActiveGEHandle = AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());

		return ActiveGEHandle.WasSuccessfullyApplied();
	}

	return false;
}


UAG_MotionWarpingComponent* AActionGameCharacter::GetAGMotionWarpingComponent() const
{
	return AGMotionWarpingComponent;
}

void AActionGameCharacter::GiveAbilities()
{
	if (HasAuthority() && AbilitySystemComponent)
	{
		for (auto DefaultAbility : CharacterData.Abilities)
		{
			AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(DefaultAbility));
		}
	}
}

void AActionGameCharacter::ApplyStartupEffects()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
		EffectContext.AddSourceObject(this);

		for (auto CharacterEffect : CharacterData.Effects)
		{
			ApplyGameplayEffectToSelf(CharacterEffect, EffectContext);
		}
	}
}

void AActionGameCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	
	AbilitySystemComponent->InitAbilityActorInfo(this, this);

	GiveAbilities();
	ApplyStartupEffects();
}

void AActionGameCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	AbilitySystemComponent->InitAbilityActorInfo(this, this);
	/*ApplyStartupEffects();*/

}

void AActionGameCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AActionGameCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}


FCharacterData AActionGameCharacter::GetCharacaterData() const
{
	return CharacterData;
}

UInventoryComponent* AActionGameCharacter::GetInventoryComponent() const
{
	return InventoryComponent;
}

void AActionGameCharacter::SetCharacaterData(const FCharacterData& InCharacterData)
{
	CharacterData = InCharacterData;
	InitFromCharacterData(CharacterData);
}

UFootstepsComponent* AActionGameCharacter::GetFootstepsComponent() const
{
	return FootstepsComponent;
}

void AActionGameCharacter::OnMaxMovementSpeedChanged(const FOnAttributeChangeData& Data)
{
	GetCharacterMovement()->MaxWalkSpeed = Data.NewValue;
}

void AActionGameCharacter::OnRep_CharacterData()
{
	InitFromCharacterData(CharacterData, true);
}

void AActionGameCharacter::InitFromCharacterData(const FCharacterData& InCharacterData, bool bFromReplication)
{

}

void AActionGameCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AActionGameCharacter, CharacterData);
	DOREPLIFETIME(AActionGameCharacter, InventoryComponent);
}
