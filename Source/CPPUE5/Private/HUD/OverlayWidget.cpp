// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/OverlayWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UOverlayWidget::SetHealthBarPercent(float Percent)
{
	if (HeathBarOverlay)
	{
		HeathBarOverlay->SetPercent(Percent);
	}
}

void UOverlayWidget::SetStaminaBarPercent(float Percent)
{
	if (StaminaBarOverlay)
	{
		StaminaBarOverlay->SetPercent(Percent);
	}
}

void UOverlayWidget::SetGold(int32 Gold)
{
	if (GoldText)
	{
		const FString String = FString::Printf(TEXT("%d"), Gold);
		const FText Text = FText::FromString(String);
		GoldText->SetText(Text);
	}
}

void UOverlayWidget::SetSouls(int32 Souls)
{
	if (SoulsText)
	{
		const FString String = FString::Printf(TEXT("%d"), Souls);
		const FText Text = FText::FromString(String);
		SoulsText->SetText(Text);
	}
}
