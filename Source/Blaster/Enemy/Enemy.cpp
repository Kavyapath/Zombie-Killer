// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy.h"
#include "Blaster/Blaster.h"
#include "Components/SkeletalMeshComponent.h"
#include "AIController.h"
#include "Navigation/PathFollowingComponent.h"
#include "NavigationData.h"
#include "Perception/PawnSensingComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Animation/AnimMontage.h"
#include "Blaster/EnemyWeapon/EnemyWeapon.h"
#include "Components/BoxComponent.h"
#include "Blaster/Widget/OverlayWidget.h"

// Sets default values
AEnemy::AEnemy()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetCharacterMovement()->bOrientRotationToMovement = true;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;

	PawnSensing = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("Pawn Sensing Component"));
	PawnSensing->SightRadius = 4000.f;
	PawnSensing->SetPeripheralVisionAngle(90.f);
}

// Called when the game starts or when spawned
void AEnemy::BeginPlay()
{
	Super::BeginPlay();


	Tags.Add(FName("Enemy"));

	OnTakeAnyDamage.AddDynamic(this,&AEnemy::ReceiveDamage);

	EnemyController = Cast<AAIController>(GetController());
	if (PawnSensing) {
		PawnSensing->OnSeePawn.AddDynamic(this, &AEnemy::PawnSeen);
	}

	SpawnDefaultWeapon();
	
}

// Called every frame
void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsInsideAttackRadius())
	{
		Attack();
		

	}
	else if (!IsInsideAttackRadius())
	{
		//ClearAttackTimer();
		ChaseTarget();
	
	}

}

// Called to bind functionality to input
void AEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AEnemy::PawnSeen(APawn* SeenPawn)
{
	const bool bShouldChaseTarget =SeenPawn->ActorHasTag(FName("Player"));

	if (bShouldChaseTarget)
	{
		CombatTarget = SeenPawn;
		ChaseTarget();
	}
}

void AEnemy::ChaseTarget()
{
	if (EnemyState != EEnemyState::EES_NoState) return;
	ClearAttackTimer();
	GetCharacterMovement()->MaxWalkSpeed = ChasingSpeed;
	MoveToTarget(CombatTarget);
}

void AEnemy::MoveToTarget(APawn* Target)
{

	
	if (EnemyController == nullptr || Target == nullptr) return;
	FAIMoveRequest MoveRequest;
	MoveRequest.SetGoalActor(Target);
	MoveRequest.SetAcceptanceRadius(AcceptanceRadius);

	EnemyController->MoveTo(MoveRequest);
}

void AEnemy::SpawnDefaultWeapon()
{
	UWorld* World = GetWorld();
	if (World)
	{
		 DefaultWeapon = World->SpawnActor<AEnemyWeapon>(WeaponClass);
		DefaultWeapon->Equip(GetMesh(), FName("WeaponSocket"), this, this);
	}
}

bool AEnemy::InTargetRange(APawn* CombatActor, double Radius)
{
	if (CombatActor == nullptr) {
		return false;
	}
	const double DistanceToTarget = (CombatActor->GetActorLocation() - GetActorLocation()).Size();
	return DistanceToTarget <= Radius;

}

void AEnemy::Attack()
{
	if (EnemyState != EEnemyState::EES_NoState) return;
	EnemyState = EEnemyState::EES_Attacking;

	
	StartAttackTimer();
}

void AEnemy::StartAttackTimer()
{

	GetWorldTimerManager().SetTimer(AttackTimer,this, &ThisClass::PlayAttackMontage, AttackTime);
}

void AEnemy::PlayAttackMontage()
{

	
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && AttackMontage)
	{

		AnimInstance->Montage_Play(AttackMontage);

	}
}

void AEnemy::ClearAttackTimer()
{

	GetWorldTimerManager().ClearTimer(AttackTimer);
}

void AEnemy::PlayHitReactMontage()
{
	EnemyState = EEnemyState::EES_HitReacting;
	GetCharacterMovement()->MaxWalkSpeed = 0;
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HitReactMontage)
	{
		
		AnimInstance->Montage_Play(HitReactMontage);
		
	}
}

void AEnemy::PlayDeathMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && DeathMontage)
	{

		AnimInstance->Montage_Play(DeathMontage);

	}
	Tags.Remove(FName("Enemy"));
}

void AEnemy::ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser)
{
	Health = FMath::Clamp(Health - Damage, 0, MaxHealth);
	PlayHitReactMontage();

	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HitParticle, DamageCauser->GetActorLocation());

	if (Health == 0.f)
	{
		GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		PlayDeathMontage();

	}
}

void AEnemy::SetEnemyStateToNoState()
{
	EnemyState = EEnemyState::EES_NoState;
	GetCharacterMovement()->MaxWalkSpeed = ChasingSpeed;
}

void AEnemy::DestroyBody()
{
	Destroy();
	DefaultWeapon->Destroy();
}

void AEnemy::SetWeaponcollisionEnabled(ECollisionEnabled::Type CollisionEnabled)
{
	if (DefaultWeapon && DefaultWeapon->GetWeaponBox()) {
		DefaultWeapon->GetWeaponBox()->SetCollisionEnabled(CollisionEnabled);
		DefaultWeapon->IgnoreActors.Empty();
	}
}

bool AEnemy::IsInsideAttackRadius()
{
	return InTargetRange(CombatTarget, AttackRadius);
}

