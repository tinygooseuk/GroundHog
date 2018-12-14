// Copyright 2018 TinyGoose Ltd., All Rights Reserved.

#pragma once

#include "GroundHog.h"
#include "GameFramework/Actor.h"
#include "SphericalLevelBoundsActor.generated.h"

// Sphere radius actor - sort of like KillZ, but with Z being any direction from a central point + radius

UCLASS()
class GROUNDHOG_API ASphericalLevelBoundsActor : public AActor
{
	GENERATED_BODY()
	
public:	
	ASphericalLevelBoundsActor();

protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float SphereRadius;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = LevelBounds, meta = (AllowPrivateAccess = "true"))
	class USphereComponent* Sphere;

	/** Spring arm for positioning the camera above the ball */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = LevelBounds, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = LevelBounds, meta = (AllowPrivateAccess = "true"))
	class URotatingMovementComponent* RotatingMovement;

	/** Camera to view the ball */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = LevelBounds, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* Camera;
};
