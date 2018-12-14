// Copyright 2018 TinyGoose Ltd., All Rights Reserved.

#include "SphericalLevelBoundsActor.h"

// Engine
#include "Camera/CameraComponent.h"
#include "Components/SphereComponent.h"
#include "EngineUtils.h"
#include "GameFramework/RotatingMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"

// Game
#include "GroundHogBall.h"

ASphericalLevelBoundsActor::ASphericalLevelBoundsActor()
	: SphereRadius(1000'00.f)
{
	PrimaryActorTick.bCanEverTick = true;

	Sphere = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere0"));
	Sphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Sphere->InitSphereRadius(SphereRadius);
	SetRootComponent(Sphere);

	// Create a camera boom attached to the root
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm0"));
	SpringArm->bDoCollisionTest = false;
	SpringArm->SetupAttachment(Sphere);
	SpringArm->TargetArmLength = SphereRadius;

	// Create a camera and attach to boom
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera0"));
	Camera->RelativeLocation = FVector(0.f, 0.f, 2000.f);
	Camera->RelativeRotation = FRotator(-30.f, 0.f, 0.f);
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);

	RotatingMovement = CreateDefaultSubobject<URotatingMovementComponent>(TEXT("RotatingMovement0"));
	RotatingMovement->SetUpdatedComponent(SpringArm);
	RotatingMovement->RotationRate = FRotator(0.f, 20.f, 0.f);
}

void ASphericalLevelBoundsActor::OnConstruction(const FTransform& Transform) 
{
	Sphere->InitSphereRadius(SphereRadius);
	SpringArm->TargetArmLength = SphereRadius;
}

void ASphericalLevelBoundsActor::BeginPlay()
{
	Super::BeginPlay();
	
}

void ASphericalLevelBoundsActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UWorld* World = GetWorld();
	GOOSE_BAIL(World);
	
	const FVector MyLoc = GetActorLocation();
	const float MaxDistSqr = SphereRadius * SphereRadius;

	// Find any balls, and kill any outside of the bounds of this "radius"
	for (TActorIterator<AGroundHogBall> It(World); It; ++It)
	{
		AGroundHogBall* Ball = *It;
		
		const FVector BallLoc = Ball->GetActorLocation();
		if (FVector::DistSquared(BallLoc, MyLoc) > MaxDistSqr)
		{
			Ball->Kill();
		}
	}	
}

