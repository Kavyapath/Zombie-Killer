// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyWeapon.h"
#include "Sound/SoundBase.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SphereComponent.h"
#include "Components/BoxComponent.h"
#include "Kismet/KismetSystemLibrary.h"

// Sets default values
AEnemyWeapon::AEnemyWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	SwordMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SowrdMesh"));
	//SwordMesh->SetupAttachment(RootComponent);
	SetRootComponent(SwordMesh);
	SwordMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	SwordMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponBox = CreateDefaultSubobject<UBoxComponent>(TEXT("WeaponBox Collision"));
	WeaponBox->SetupAttachment(GetRootComponent());
	WeaponBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	WeaponBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);

	StartTrace = CreateDefaultSubobject<USceneComponent>(TEXT("Box Trace Start"));
	StartTrace->SetupAttachment(RootComponent);
	EndTrace = CreateDefaultSubobject<USceneComponent>(TEXT("Box Trace End"));
	EndTrace->SetupAttachment(RootComponent);

}

// Called when the game starts or when spawned
void AEnemyWeapon::BeginPlay()
{
	Super::BeginPlay();
	WeaponBox->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnBoxOverlap);
	
}

void AEnemyWeapon::OnBoxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherActorComponent, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	FHitResult BoxHit;
	BoxTrace(BoxHit);

	UGameplayStatics::ApplyDamage(
		BoxHit.GetActor(),
		Damage,
		GetInstigator()->GetController(),// using GetInstigator() we will get the pawn owner of this weapon and from the pawn we can access its controller using GetController()
		this,
		UDamageType::StaticClass()
	);

}

// Called every frame
void AEnemyWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AEnemyWeapon::BoxTrace(FHitResult& BoxHit)
{
	const FVector Start = StartTrace->GetComponentLocation();//world space location
	const FVector End = EndTrace->GetComponentLocation();

	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(this);
	ActorsToIgnore.Add(GetOwner());
	
	for (AActor* Actor : IgnoreActors) {
		ActorsToIgnore.AddUnique(Actor);
	}



	UKismetSystemLibrary::BoxTraceSingle(
		this,
		Start,
		End,
		BoxTraceExtent,
		StartTrace->GetComponentRotation(),//Rotation of start trace
		ETraceTypeQuery::TraceTypeQuery1,
		false,
		ActorsToIgnore,
		bShowDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None,
		BoxHit,
		true
	);
	IgnoreActors.AddUnique(BoxHit.GetActor());
}

void AEnemyWeapon::Equip(USceneComponent* InParent, FName InSocketName, AActor* NewOwner, APawn* NewInstigator)
{
	SetOwner(NewOwner);
	SetInstigator(NewInstigator);
	AttachMeshToSocket(InParent, InSocketName);
}

void AEnemyWeapon::AttachMeshToSocket(USceneComponent* InParent, const FName& InSocketName)
{
	FAttachmentTransformRules TransformRules(EAttachmentRule::SnapToTarget, true);//they directly called or initialzie using Structs constructor
	SwordMesh->AttachToComponent(InParent, TransformRules, InSocketName);
	
}

