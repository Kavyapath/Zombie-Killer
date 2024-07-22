// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLASTER_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCombatComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	bool bFireButtonPressed;
	bool bCanFire=true;

	FTimerHandle FireTimer;
	UPROPERTY(EditAnywhere)
	float FireDelay=0.1;
	bool CanFire();
	void StartFireTimer();
	void FireTimerFinished();
	void FireButtonPressed(bool bPressed);
	void Fire();

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	friend class ATank;

private:
	UPROPERTY()
	class ATank* Tank;
		
};
