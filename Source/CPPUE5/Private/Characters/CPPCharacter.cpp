// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/CPPCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/InputComponent.h"
#include "Components/AttributeComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Items/Items.h"
#include "Items/Weapons/Weapons.h"
#include "Items/Soul.h"
#include "Items/Treasure.h"
#include "Animation/AnimMontage.h"
#include "HUD/GameHUD.h"
#include "HUD/OverlayWidget.h"

ACPPCharacter::ACPPCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 400.f, 0.f);
	GetCharacterMovement()->MaxAcceleration = 2048.f;

	GetMesh()->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	GetMesh()->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);
	GetMesh()->SetGenerateOverlapEvents(true);

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 300.f;
	
	ViewCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ViewCamera"));
	ViewCamera->SetupAttachment(CameraBoom);

	AutoPossessPlayer = EAutoReceiveInput::Player0;

}

void ACPPCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (Attributes && OverlayWidget)
	{
		Attributes->RegenStamina(DeltaTime);
		OverlayWidget->SetStaminaBarPercent(Attributes->GetStaminaPercent());
	}
}

void ACPPCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ACPPCharacter::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ACPPCharacter::Look);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACPPCharacter::Jump);
		EnhancedInputComponent->BindAction(EquipAction, ETriggerEvent::Triggered, this, &ACPPCharacter::Equip);
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Triggered, this, &ACPPCharacter::Attack);
		EnhancedInputComponent->BindAction(DodgeAction, ETriggerEvent::Triggered, this, &ACPPCharacter::Dodge);
	}
}

void ACPPCharacter::Jump()
{
	if (IsUnoccupied())
	{
		Super::Jump();
	}
}

bool ACPPCharacter::IsUnoccupied()
{
	return ActionState == EActionState::EAS_Unoccupied;
}

float ACPPCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	HandleDamage(DamageAmount);
	SetHUDHealth();
	return DamageAmount;
}

void ACPPCharacter::GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter)
{
	Super::GetHit_Implementation(ImpactPoint, Hitter);

	SetWeaponCollisionEnabled(ECollisionEnabled::NoCollision);
	if (Attributes && Attributes->GetHealthPercent() > 0.f)
	{
		ActionState = EActionState::EAS_HitReaction;
	}
}

void ACPPCharacter::SetOverlappingItem(AItems* Item)
{
	OverlappingItem = Item;
}

void ACPPCharacter::AddSouls(ASoul* Soul)
{
	if (Attributes && OverlayWidget)
	{
		Attributes->AddSouls(Soul->GetSouls());
		OverlayWidget->SetSouls(Attributes->GetSouls());
	}
}

void ACPPCharacter::AddGold(ATreasure* Treasure)
{
	if (Attributes && OverlayWidget)
	{
		Attributes->AddGold(Treasure->GetGold());
		OverlayWidget->SetGold(Attributes->GetGold());
	}
}

void ACPPCharacter::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (PlayerController)
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(CharacterMappingContext, 0);
		}
	}
	InitializeOverlay();
	Tags.Add(FName("EngageableTarget"));

}

void ACPPCharacter::Move(const FInputActionValue& Value)
{
	if (ActionState != EActionState::EAS_Unoccupied) return;
	const FVector2D DirectionValue = Value.Get<FVector2D>();
	if (GetController())
	{
		const FRotator ControlRotation = GetControlRotation();
		const FRotator YawRotation(0.f, ControlRotation.Yaw, 0.f);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(ForwardDirection, DirectionValue.Y);

		const FVector SideDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(SideDirection, DirectionValue.X);
	}
}

void ACPPCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D LookAxisValue = Value.Get<FVector2D>();
	if (GetController())
	{
		AddControllerYawInput(LookAxisValue.X);
		AddControllerPitchInput(LookAxisValue.Y);
	}
}

void ACPPCharacter::Equip(const FInputActionValue& Value)
{
	AWeapons* OverlappingWeapon = Cast<AWeapons>(OverlappingItem);
	//if (OverlappingWeapon)
	if (OverlappingWeapon && !EquipWeapon)
	{
		if (OverlappingWeapon)
		{
			OverlappingWeapon->Destroy();

		}
		CharacterEquipWeapon(OverlappingWeapon);
	}
	else
	{
		if (CanSheathe())
		{
			Sheathe();
		}
		else if (CanDraw())
		{
			Draw();
		}
	}
}

void ACPPCharacter::Dodge(const FInputActionValue& Value)
{
	if (IsOccupied() || !HasEnoughStamina()) return;

	PlayDodgeMontage();
	ActionState = EActionState::EAS_Dodge;
	if (Attributes && OverlayWidget)
	{
		Attributes->UseStamina(Attributes->GetDodgeCost());
		OverlayWidget->SetStaminaBarPercent(Attributes->GetStaminaPercent());
	}
}

void ACPPCharacter::Attack()
{
	Super::Attack();

	if (CanAttack())
	{
		PlayAttackMontage();
		ActionState = EActionState::EAS_Attacking;
	}
}

void ACPPCharacter::CharacterEquipWeapon(AWeapons* Weapon)
{
	Weapon->Equip(GetMesh(), FName("RightHandSocket"), this, this);
	CharacterState = ECharacterState::ECS_EquippedTwoHWeapon;
	OverlappingItem = nullptr;
	EquipWeapon = Weapon;
}

void ACPPCharacter::AttackEnd()
{
	ActionState = EActionState::EAS_Unoccupied;
}

void ACPPCharacter::DodgeEnd()
{
	Super::DodgeEnd();

	ActionState = EActionState::EAS_Unoccupied;
}

bool ACPPCharacter::CanAttack()
{

	return ActionState == EActionState::EAS_Unoccupied &&
		CharacterState != ECharacterState::ECS_Unequipped;
}

bool ACPPCharacter::CanSheathe()
{
	return ActionState == EActionState::EAS_Unoccupied && 
		CharacterState != ECharacterState::ECS_Unequipped;
}

bool ACPPCharacter::CanDraw()
{
	return ActionState == EActionState::EAS_Unoccupied &&
		CharacterState == ECharacterState::ECS_Unequipped && 
		EquipWeapon;
}

void ACPPCharacter::Sheathe()
{
	PlayEquipMontage(FName("SheatheSword"));
	CharacterState = ECharacterState::ECS_Unequipped;
	ActionState = EActionState::EAS_EquippingWeapon;
}

void ACPPCharacter::Draw()
{
	PlayEquipMontage(FName("DrawSword"));
	CharacterState = ECharacterState::ECS_EquippedTwoHWeapon;
	ActionState = EActionState::EAS_EquippingWeapon;
}

void ACPPCharacter::AttachWeaponToBack()
{
	if (EquipWeapon)
	{
		EquipWeapon->AttachMeshToSocket(GetMesh(), FName("Status"));
	}
}

void ACPPCharacter::AttachWeaponToHand()
{
	if (EquipWeapon)
	{
		EquipWeapon->AttachMeshToSocket(GetMesh(), FName("RightHandSocket"));
	}
}

void ACPPCharacter::PlayEquipMontage(const FName& SectionName)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && EquipMontage)
	{
		AnimInstance->Montage_Play(EquipMontage);
		AnimInstance->Montage_JumpToSection(SectionName, EquipMontage);
	}
}

void ACPPCharacter::Die_Implementation()
{
	Super::Die_Implementation();

	ActionState = EActionState::EAS_Dead;
}

bool ACPPCharacter::HasEnoughStamina()
{
	return Attributes && Attributes->GetStamina() > Attributes->GetDodgeCost();
}

bool ACPPCharacter::IsOccupied()
{
	return ActionState != EActionState::EAS_Unoccupied;
}

void ACPPCharacter::FinishEquipping()
{
	ActionState = EActionState::EAS_Unoccupied;
}

void ACPPCharacter::HitReactEnd()
{
	ActionState = EActionState::EAS_Unoccupied;
}

void ACPPCharacter::InitializeOverlay()
{
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (PlayerController)
	{
		AGameHUD* GameHUD = Cast<AGameHUD>(PlayerController->GetHUD());
		if (GameHUD)
		{
			OverlayWidget = GameHUD->GetGameOverlay();
			if (OverlayWidget && Attributes)
			{
				OverlayWidget->SetHealthBarPercent(Attributes->GetHealthPercent());
				OverlayWidget->SetStaminaBarPercent(1.f);
				OverlayWidget->SetGold(0);
				OverlayWidget->SetSouls(0);
			}
		}
	}
}

void ACPPCharacter::SetHUDHealth()
{
	if (OverlayWidget && Attributes)
	{
		OverlayWidget->SetHealthBarPercent(Attributes->GetHealthPercent());
	}
}
