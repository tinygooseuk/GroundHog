// Copyright 2018 TinyGoose Ltd., All Rights Reserved.

#include "SplineMovementActor.h"

#include "Components/SplineComponent.h"

ASplineMovementActor::ASplineMovementActor()
	: ActorToMove(nullptr)
	, MovementSpeed(100.f)
	, bGoBackwards(false)
	, SplinePosition(0.f)
{
	PrimaryActorTick.bCanEverTick = true;

	SplineComponent = CreateDefaultSubobject<USplineComponent>(TEXT("SplineComp0"));
	RootComponent = SplineComponent;
}

void ASplineMovementActor::OnConstruction(const FTransform& Transform) 
{
	if (ActorToMove)
	{
		ActorToMove->SetActorLocation(SplineComponent->GetLocationAtDistanceAlongSpline(0.f, ESplineCoordinateSpace::World));
	}
}

void ASplineMovementActor::BeginPlay()
{
	Super::BeginPlay();
	
}

void ASplineMovementActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!ActorToMove)
	{
		return;
	}

	// Move actor along spline and perhaps reverse
	float Dir = bGoBackwards ? -1.f : 1.f;
	SplinePosition += MovementSpeed * DeltaTime * Dir;

	if (SplinePosition > SplineComponent->GetSplineLength())
	{
		bGoBackwards = true;
		SplinePosition = SplineComponent->GetSplineLength();
	}
	else if (SplinePosition <= 0.f)
	{
		bGoBackwards = false;
		SplinePosition = 0.f;
	}
	
	// Actually move actor!
	FVector NewPosition = SplineComponent->GetLocationAtDistanceAlongSpline(SplinePosition, ESplineCoordinateSpace::World);
	ActorToMove->SetActorLocation(NewPosition);
}