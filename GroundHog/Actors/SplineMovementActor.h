// Copyright 2018 TinyGoose Ltd., All Rights Reserved.

#pragma once

#include "GroundHog.h"
#include "GameFramework/Actor.h"
#include "SplineMovementActor.generated.h"

UCLASS()
class GROUNDHOG_API ASplineMovementActor : public AActor
{
	GENERATED_BODY()
	
public:	
	ASplineMovementActor();

	virtual void OnConstruction(const FTransform& Transform);

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

public:
	///////////////////////// Components /////////////////////////
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	class USplineComponent* SplineComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	class AActor* ActorToMove;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float MovementSpeed;

private:
	///////////////////////// State /////////////////////////
	bool bGoBackwards;
	float SplinePosition;
};
