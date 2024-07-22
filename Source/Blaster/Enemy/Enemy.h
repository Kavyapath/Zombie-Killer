// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "EnemyState.h"
#include "Enemy.generated.h"

UCLASS()
class BLASTER_API AEnemy : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AEnemy();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
	void ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser);

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	void PlayHitReactMontage();
	void PlayDeathMontage();

		UFUNCTION(BlueprintCallable)
	void SetEnemyStateToNoState();

	UFUNCTION(BlueprintCallable)
	void DestroyBody();

	UFUNCTION(BlueprintCallable)
	void SetWeaponcollisionEnabled(ECollisionEnabled::Type CollisionEnabled);

private:
	UPROPERTY(VisibleAnywhere)
	class UPawnSensingComponent* PawnSensing;
	UPROPERTY(EditAnywhere)
	float ChasingSpeed = 500.f;
	UPROPERTY(EditAnywhere)
	float AcceptanceRadius = 100.f;

	UPROPERTY(EditAnywhere)
	float  AttackRadius = 200.f;
	UFUNCTION()
	void PawnSeen(APawn* SeenPawn);
	void ChaseTarget();
	void MoveToTarget(APawn* Target);
	void SpawnDefaultWeapon();
	class AAIController* EnemyController;
	EEnemyState EnemyState;

	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage* HitReactMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage* DeathMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage* AttackMontage;

	UPROPERTY(EditAnywhere)
	float Health = 100.f;

	UPROPERTY(EditAnywhere)
	float MaxHealth = 100.f;

	UPROPERTY(EditAnyWhere)
	TSubclassOf<class AEnemyWeapon> WeaponClass;

	AEnemyWeapon* DefaultWeapon;
	bool IsInsideAttackRadius();
	bool InTargetRange(APawn* CombatActor, double Radius);

	void Attack();

	void StartAttackTimer();
	void PlayAttackMontage();
	void ClearAttackTimer();

	FTimerHandle AttackTimer;
	float AttackTime = 1.f;

	UPROPERTY(EditAnyWhere)
	UParticleSystem* HitParticle;
	float killCount = 0.f;

	UPROPERTY()
	class UOverlayWidget* Overlay;
protected:
	APawn* CombatTarget;
};
