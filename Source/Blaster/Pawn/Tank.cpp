// Fill out your copyright notice in the Description page of Project Settings.


#include "Tank.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Components/InputComponent.h"
#include "InputMappingContext.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Blaster/Weapon/Projectile.h"
#include "Blaster/Component/CombatComponent.h"
#include "Components/CapsuleComponent.h"
#include "Sound/SoundCue.h"
#include "Blaster/Widget/OverlayWidget.h"
#include "Sound/SoundCue.h"

// Sets default values
ATank::ATank()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CapsuleComp = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule Collider")); //and function to create any component by giving its type in template it is a template fun which will adapt to the type that we want
	RootComponent = CapsuleComp;
	CapsuleComp->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

	BaseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Base Mesh"));
	BaseMesh->SetupAttachment(CapsuleComp);//to attach our mesh component to the capsule comp(root component)
	

	BaseMesh->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	BaseMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);

	BaseMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	BaseMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);
	BaseMesh->SetGenerateOverlapEvents(true);

	

	TurretMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Turret Mesh"));
	TurretMesh->SetupAttachment(BaseMesh);
	TurretMesh->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	TurretMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);

	TurretMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	TurretMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);
	TurretMesh->SetGenerateOverlapEvents(true);

	ProjectileSpawnPoint = CreateDefaultSubobject<USceneComponent>(TEXT("Spawn Point"));
	ProjectileSpawnPoint->SetupAttachment(TurretMesh);

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Spring Arm"));
	SpringArm->SetupAttachment(RootComponent);
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);

	Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("combat"));

}

// Called when the game starts or when spawned
void ATank::BeginPlay()
{
	Super::BeginPlay();
	if (const APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
	Tags.Add(FName("Player"));
	OnTakeAnyDamage.AddDynamic(this, &ThisClass::ReceiveDamage);
	PlayerControllerRef = Cast<APlayerController>(GetController());
	AddWidget();
	Overlay->SetHealthPercent(Health, MaxHealth);
}

void ATank::Move(const FInputActionValue& Value)
{
	const FVector2D AxisValue = Value.Get<FVector2D>();
	FVector DeltaLocation(0.f);
	//X=Deltatime*Value*speed to control the framerate during movement because delta time is the difference between initially and final tick function call time

	DeltaLocation.X = AxisValue.Y * Speed * UGameplayStatics::GetWorldDeltaSeconds(this);
	AddActorLocalOffset(DeltaLocation, true);
}

void ATank::Look(const FInputActionValue& Value)
{
	
	const FVector2D AxisValue = Value.Get<FVector2D>();
	if (GetController()) {
		
		AddControllerYawInput(AxisValue.X * RotationRate * GetWorld()->GetDeltaSeconds());
	}
}

void ATank::Turn(const FInputActionValue& Value)
{
	const FVector2D MovementVector = Value.Get<FVector2D>();
	FRotator DeltaRotation = FRotator::ZeroRotator;
	//yaw=Deltatime*Value*turnspeed //like x,y,z yaw means we want rotation in the z axis
	DeltaRotation.Yaw = MovementVector.X * TurnRate * UGameplayStatics::GetWorldDeltaSeconds(this);

	AddActorLocalRotation(DeltaRotation, true);

}

void ATank::RotateTurret(FVector LookAtTarget)
{
	FVector ToTarget = LookAtTarget - TurretMesh->GetComponentLocation();
	FRotator LookAtRotation = FRotator(0.f, ToTarget.Rotation().Yaw, 0.f);//for settin the fRotator
	TurretMesh->SetWorldRotation(
		FMath::RInterpTo(TurretMesh->GetComponentRotation(),
			LookAtRotation,
			UGameplayStatics::GetWorldDeltaSeconds(this),
			300.f));
}

void ATank::Fire()
{
	FVector Location = ProjectileSpawnPoint->GetComponentLocation();//function and the object to find the location of the projectile launch

	FRotator Rotation = ProjectileSpawnPoint->GetComponentRotation();
	//now instead of our debug sphere our projectile will be launching whwn we click on the left fire button
	//now we want to spawn the projectile class as the actor
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = this;
	UWorld* World = GetWorld();
	if (World) {
		World->SpawnActor<AProjectile>(ProjectileClass,
			Location,
			Rotation,
			SpawnParams
		);
	}

	if (FireEffect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(this,
			FireEffect,
			Location);
	}
	if (FireSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this,
			FireSound,
			Location);
	}
	
}

// Called every frame
void ATank::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FHitResult HitResult;



	if (PlayerControllerRef) {
		PlayerControllerRef->GetHitResultUnderCursor(ECollisionChannel::ECC_Visibility,
			false,
			HitResult);
		/*DrawDebugSphere(GetWorld(),
			HitResult.ImpactPoint,//GetActorLocation() + FVector(0.f, 0.f, 200.f),// to make mouse curser movement in the game getting actor location  +FVector offset that at which distance we wank to move the curser in the x direction
			25.f,
			12,
			FColor::Red,
			false,
			-1.f);*/
	}

	RotateTurret(HitResult.ImpactPoint);

}

// Called to bind functionality to input
void ATank::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);


	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ThisClass::Move);
		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ThisClass::Look);

		EnhancedInputComponent->BindAction(TurnAction, ETriggerEvent::Triggered, this, &ThisClass::Turn);

		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, this, &ThisClass::FireButtonPressed);
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Completed, this, &ThisClass::FireButtonReleased);

		EnhancedInputComponent->BindAction(FireRocketAction, ETriggerEvent::Triggered, this, &ThisClass::FireRocket);
	}
}

void ATank::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (Combat)
	{
		Combat->Tank = this;
	}
}

void ATank::AddWidget()
{
	APlayerController* PlayerController = Cast<APlayerController>(Controller);
	if (PlayerController && OverlayClass)
	{
		Overlay=CreateWidget<UOverlayWidget>(PlayerController, OverlayClass);
		Overlay->AddToViewport();
	}
}


void ATank::FireButtonPressed()
{
	if (Combat)
	{
	
		Combat->FireButtonPressed(true);
	}
}

void ATank::FireButtonReleased()
{
	Combat->FireButtonPressed(false);
}

void ATank::FireRocket()
{

	FVector Location = ProjectileSpawnPoint->GetComponentLocation();//function and the object to find the location of the projectile launch

	FRotator Rotation = ProjectileSpawnPoint->GetComponentRotation();
	//now instead of our debug sphere our projectile will be launching whwn we click on the left fire button
	//now we want to spawn the projectile class as the actor
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = this;
	UWorld* World = GetWorld();
	if (World) {
		World->SpawnActor<AProjectile>(ProjectileClassRocket,
			Location,
			Rotation,
			SpawnParams
		);
	}
}

void ATank::ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser)
{
	Health = FMath::Clamp(Health - Damage, 0, MaxHealth);
	Overlay->SetHealthPercent(Health, MaxHealth);
	
	

	if (HitCameraShakeClass) {
		GetWorld()->GetFirstPlayerController()->ClientStartCameraShake(HitCameraShakeClass);
	}
	if (HitSound)
	{

		UGameplayStatics::PlaySoundAtLocation(this, HitSound, GetActorLocation());
	}

	if (Health == 0.f)
	{

		if (DeathCameraShakeClass) {
			GetWorld()->GetFirstPlayerController()->ClientStartCameraShake(DeathCameraShakeClass);
		}

		if (DeathEffect)
		{
			UGameplayStatics::SpawnEmitterAtLocation(this, DeathEffect, GetActorLocation());
		}
		Destroy();

	}
	
}