// Fill out your copyright notice in the Description page of Project Settings.


#include "OverlayWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UOverlayWidget::SetHealthPercent(float Health, float MaxHealth)
{
	if (HealthBar)
	{
		const float HealthPercent = Health / MaxHealth;
		HealthBar->SetPercent(HealthPercent);
	}
}


