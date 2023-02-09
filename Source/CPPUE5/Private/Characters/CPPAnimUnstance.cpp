// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/CPPAnimUnstance.h"
#include "Characters/CPPCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

void UCPPAnimUnstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	CPPCharacter = Cast<ACPPCharacter>(TryGetPawnOwner());
	if (CPPCharacter)
	{
		CPPCharacterMovement = CPPCharacter->GetCharacterMovement();
	}
}

void UCPPAnimUnstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);
	if (CPPCharacterMovement)
	{
		GroundSpeed = UKismetMathLibrary::VSizeXY(CPPCharacterMovement->Velocity);
		GroundAcceleration = UKismetMathLibrary::VSizeXY(CPPCharacterMovement->GetCurrentAcceleration());
		if (GroundAcceleration > 0)
		{
			IsAccelerating = true;
		}
		else 
		{
			IsAccelerating = false;
		}
		IsFalling = CPPCharacterMovement->IsFalling();
		CharacterState = CPPCharacter->GetCharacterState();
		ActionState = CPPCharacter->GetActionState();
		DeathPose = CPPCharacter->GetDeathPose();
	}
}
