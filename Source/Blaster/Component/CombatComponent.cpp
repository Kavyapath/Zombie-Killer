// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"
#include "Blaster/Pawn/Tank.h"

// Sets default values for this component's properties
UCombatComponent::UCombatComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

bool UCombatComponent::CanFire()
{
	if (Tank == nullptr) return false;
	return bCanFire;
}

void UCombatComponent::StartFireTimer()
{
	if (Tank == nullptr) return;

	Tank->GetWorldTimerManager().SetTimer(
	FireTimer,
		this,
		&UCombatComponent::FireTimerFinished,
		FireDelay);
}

void UCombatComponent::FireTimerFinished()
{
	if (Tank == nullptr) return;
	bCanFire = true;
	if (bFireButtonPressed)
	{
		Fire();
	}
}

void UCombatComponent::FireButtonPressed(bool bPressed)
{
	bFireButtonPressed = bPressed;
	if (bFireButtonPressed)
	{
		Fire();
	}
}

void UCombatComponent::Fire()
{
	if (CanFire())
	{
		bCanFire = false;
		Tank->Fire();
		StartFireTimer();
	}
}


// Called every frame
void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

