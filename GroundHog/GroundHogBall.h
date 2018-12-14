// Copyright 2018 TinyGoose Ltd., All Rights Reserved.

#pragma once

#include "GroundHog.h"
#include "GameFramework/Pawn.h"
#include "GroundHogBall.generated.h"

UCLASS(config=Game)
class AGroundHogBall : public APawn
{
	GENERATED_BODY()

public:
	AGroundHogBall();
		
protected:
	// AActor interface
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void NotifyHit(class UPrimitiveComponent* MyComp, class AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit) override;
	// End of AActor interface
	
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;
	// End of APawn interface

	void MoveRight(float Val);
	void MoveForward(float Val);
	void CameraMoveRight(float Val);
	void CameraMoveUp(float Val);

	/** Handle jump action. */
	void Jump();

	/** Handler for when a touch input begins. */
	void TouchStarted(ETouchIndex::Type FingerIndex, FVector Location);

public:
	UFUNCTION(BlueprintCallable)
	bool GetBlinking() const;

	UFUNCTION(BlueprintCallable)
	void SetBlinking(bool bBlinking);

	UFUNCTION(BlueprintCallable)
	void Kill();

	///////////////////////// Properties /////////////////////////
	/** Vertical impulse to apply when pressing jump */
	UPROPERTY(EditAnywhere, Category=Ball)
	float JumpImpulse;

	/** Torque to apply when trying to roll ball */
	UPROPERTY(EditAnywhere, Category=Ball)
	float RollTorque;

	/** Torque to apply when trying to roll ball */
	UPROPERTY(EditAnywhere, Category = Ball)
	float AirBumpForce;
		
private:
	UFUNCTION()
	void HandleRespawn();

	UFUNCTION()
	void QuitGame();

	void UpdateBlinking(float DeltaSeconds);

	FQuat GetUpQuaternion() const;

	///////////////////////// Statics /////////////////////////
	static constexpr float MaxCameraRotation_Vertical = 30.f;
	
	///////////////////////// Components /////////////////////////
	/** StaticMesh used for the ball */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Ball, meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* Ball;

	/** Spring arm for positioning the camera above the ball */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Ball, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* SpringArm;

	/** Camera to view the ball */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Ball, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* Camera;

	///////////////////////// Material instances /////////////////////////
	class UMaterialInstanceDynamic* HogMatInstance;

	///////////////////////// UI /////////////////////////
	class UClass* UMG_SpawnCountdown;

	///////////////////////// Sounds ////////////////////
	class USoundWave* JumpSound;
	class USoundWave* FallSound;
	class USoundWave* SpawnSound;

	///////////////////////// State /////////////////////////
	/** Indicates whether we can currently jump, use to prevent double jumping */
	bool bCanJump;

	bool bKilled;
	
	bool bBlinking;
	float BlinkTimer;

	float SquashAmount;

	FVector GravityDirection;
	FRotator CameraOffset;
};
