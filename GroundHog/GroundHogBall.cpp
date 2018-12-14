// Copyright 2018 TinyGoose Ltd., All Rights Reserved.

#include "GroundHogBall.h"

// Engine
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"
#include "Camera/CameraComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InputComponent.h"
#include "Engine/CollisionProfile.h"
#include "Engine/StaticMesh.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PhysicsVolume.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Sound/SoundWave.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

// Game
#include "Actors/SphericalLevelBoundsActor.h"

namespace HogConsts
{
	constexpr float BLINK_TIME = 0.2f;
	constexpr float TIME_BETWEEN_BLINK_MIN = 1.5f;
	constexpr float TIME_BETWEEN_BLINK_MAX = 3.5f;
}

AGroundHogBall::AGroundHogBall()
	: GravityDirection(FVector(0.f, 0.f, -1.f))
	, SquashAmount(0.f)
	, bKilled(false)
	, bBlinking(false)
	, BlinkTimer(0.f)
{
	PrimaryActorTick.bCanEverTick = true;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> BallMesh(TEXT("/Game/Meshes/SM_Hog.SM_Hog"));

	UMG_SpawnCountdown = FIND_CLASS(UMG_SpawnCountdown, "UMG");
	JumpSound = FIND_RESOURCE(SoundWave, Jump, "Sounds");
	FallSound = FIND_RESOURCE(SoundWave, Fall, "Sounds");
	SpawnSound = FIND_RESOURCE(SoundWave, Spawn, "Sounds");
		
	// Create mesh component for the ball
	Ball = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Ball0"));
	Ball->SetStaticMesh(BallMesh.Object);
	Ball->BodyInstance.SetCollisionProfileName(UCollisionProfile::PhysicsActor_ProfileName);
	Ball->SetSimulatePhysics(true);
	Ball->SetAngularDamping(0.4f);
	Ball->SetLinearDamping(0.1f);
	Ball->BodyInstance.MassScale = 3.5f;
	Ball->BodyInstance.MaxAngularVelocity = 800.0f;
	Ball->SetNotifyRigidBodyCollision(true);
	Ball->SetEnableGravity(false);
	RootComponent = Ball;

	// Create a camera boom attached to the root (ball)
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm0"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->bDoCollisionTest = true;
	SpringArm->ProbeSize = 20.f;

	SpringArm->bAbsoluteRotation = true; // Rotation of the ball should not affect rotation of boom
	SpringArm->TargetArmLength = 1000.f;
	SpringArm->bEnableCameraRotationLag = true;
	SpringArm->CameraRotationLagSpeed = 4.f;
	SpringArm->bEnableCameraLag = true;
	SpringArm->CameraLagSpeed = 6.f;

	// Create a camera and attach to boom
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera0"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false; // We don't want the controller rotating the camera

	// Set up forces
	RollTorque = 50'000'000.0f;
	AirBumpForce = 5'000.0f;
	JumpImpulse = 1'000'000.0f;//1500000.0f;
	bCanJump = true; // Start being able to jump
}

void AGroundHogBall::BeginPlay()
{
	Super::BeginPlay();

	// Instance our material - so that we can blink!
	UMaterialInterface* CurrentMat = Ball->GetMaterial(0);
	GOOSE_BAIL(CurrentMat);
	
	HogMatInstance = UMaterialInstanceDynamic::Create(CurrentMat, this);
	Ball->SetMaterial(0, HogMatInstance);

	// Play spawn sound!
	UGameplayStatics::PlaySoundAtLocation(this, SpawnSound, GetActorLocation());
}

void AGroundHogBall::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UWorld* World = GetWorld();
	GOOSE_BAIL(World);

	APhysicsVolume* PhysVolume = World->GetDefaultPhysicsVolume();
	GOOSE_BAIL(PhysVolume);

	// Add gravity
	float GravStrength = FMath::Abs(PhysVolume->GetGravityZ());
	Ball->AddForce(GravityDirection * GravStrength, NAME_None, true);
	
	// Rotate camera
	const FQuat UpQuat = GetUpQuaternion();
	SpringArm->SetRelativeRotation(UpQuat * CameraOffset.Quaternion() * FRotator(-45.f, 0.f, 0.f).Quaternion());

	// Update eye blink
	UpdateBlinking(DeltaSeconds);
}

void AGroundHogBall::NotifyHit(class UPrimitiveComponent* MyComp, class AActor* Other, class UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
	Super::NotifyHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);

	bCanJump = true;

	// Blink if required
	if (NormalImpulse.Size() > 40'000.f)
	{	
		SetBlinking(true);
		BlinkTimer = HogConsts::BLINK_TIME * 2.f;
	}

	// Change world gravity to match the new floor.
	// Yes, this is the magic folks!
	GravityDirection = -HitNormal;
}

void AGroundHogBall::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// set up gameplay key bindings
	PlayerInputComponent->BindAxis("MoveRight", this, &AGroundHogBall::MoveRight);
	PlayerInputComponent->BindAxis("MoveForward", this, &AGroundHogBall::MoveForward);

	PlayerInputComponent->BindAxis("CameraMoveRight", this, &AGroundHogBall::CameraMoveRight);
	PlayerInputComponent->BindAxis("CameraMoveUp", this, &AGroundHogBall::CameraMoveUp);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AGroundHogBall::Jump);

	// Quit game
	PlayerInputComponent->BindAction("Quit", IE_Pressed, this, &AGroundHogBall::QuitGame);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &AGroundHogBall::TouchStarted);
}

void AGroundHogBall::MoveRight(float Val)
{
	FQuat Quat = GetUpQuaternion();
	const FQuat CamTurn = FRotator(0.f, CameraOffset.Yaw, 0.f).Quaternion();

	// Apply camera quaternion to movement
	Quat *= CamTurn;

	if (!bCanJump)
	{
		// Add small mid-air impulse
		Ball->AddImpulse(Quat.RotateVector(FVector::RightVector * AirBumpForce * Val));
	}
	else
	{
		// Spin if on ground
		const FVector Torque = FVector(-1.f * Val * RollTorque, 0.f, 0.f);
		Ball->AddTorqueInRadians(Quat.RotateVector(Torque));
	}
}

void AGroundHogBall::MoveForward(float Val)
{
	FQuat Quat = GetUpQuaternion();
	const FQuat CamTurn = FRotator(0.f, CameraOffset.Yaw, 0.f).Quaternion();

	// Apply camera quaternion to movement
	Quat *= CamTurn;
	
	if (!bCanJump)
	{
		// Add small mid-air impulse
		Ball->AddImpulse(Quat.RotateVector(FVector::ForwardVector * AirBumpForce * Val));
	}
	else
	{
		// Spin if on ground
		const FVector Torque = FVector(0.f, Val * RollTorque, 0.f);
		Ball->AddTorqueInRadians(Quat.RotateVector(Torque));
	}
}
void AGroundHogBall::CameraMoveRight(float Val)
{
	CameraOffset.Yaw += Val * 2.f;
}
void AGroundHogBall::CameraMoveUp(float Val)
{
	CameraOffset.Pitch += Val * 2.f;// Val * AGroundHogBall::MaxCameraRotation_Vertical;
	CameraOffset.Pitch = FMath::Max(FMath::Min(CameraOffset.Pitch, 45.f), -45.f);
}

void AGroundHogBall::Jump()
{
	if (bCanJump)
	{
		// Add impulse towards whichever way is "up"
		const FQuat Quat = GetUpQuaternion();
		const FVector Impulse = FVector(0.f, 0.f, JumpImpulse);
		
		Ball->AddImpulse(Quat.RotateVector(Impulse));
		bCanJump = false;

		// Jump sound
		UGameplayStatics::PlaySoundAtLocation(this, JumpSound, GetActorLocation());
	}
}

void AGroundHogBall::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
	Jump();
}

bool AGroundHogBall::GetBlinking() const
{
	return bBlinking;
}
void AGroundHogBall::SetBlinking(bool bInBlinking)
{
	bBlinking = bInBlinking;
	
	GOOSE_BAIL(HogMatInstance); 

	static const FName NAME_Blinking(TEXT("Blink"));
	HogMatInstance->SetScalarParameterValue(NAME_Blinking, bBlinking ? 1.f : 0.f);
}

void AGroundHogBall::Kill()
{
	if (bKilled)
	{
		return;
	}

	bKilled = true;

	UWorld* World = GetWorld();
	GOOSE_BAIL(World);

	APlayerController* PC = Cast<APlayerController>(GetController());
	GOOSE_BAIL(PC);

	// Play sound
	UGameplayStatics::PlaySoundAtLocation(this, FallSound, GetActorLocation());

	// Change to overview cam
	TActorIterator<ASphericalLevelBoundsActor> It(World);
	if (It)
	{
		AActor* BoundsActor = Cast<AActor>(*It);
		
		FViewTargetTransitionParams TransParams;
		TransParams.BlendExp = 3;
		TransParams.BlendFunction = EViewTargetBlendFunction::VTBlend_EaseInOut;
		TransParams.BlendTime = 1.0f;
		PC->SetViewTarget(BoundsActor, TransParams);
	}

	// Schedule respawn
	FTimerDelegate TimerCallback;
	TimerCallback.BindUFunction(this, TEXT("HandleRespawn"));

	FTimerHandle Handle;
	World->GetTimerManager().SetTimer(Handle, TimerCallback, 5.0f, false);

	// Add widgets
	UUserWidget* Countdown = CreateWidget<UUserWidget>(PC, UMG_SpawnCountdown);
	GOOSE_BAIL(Countdown);
	Countdown->AddToViewport();
}

void AGroundHogBall::HandleRespawn()
{
	UWorld* World = GetWorld();
	GOOSE_BAIL(World);

	APlayerController* PC = Cast<APlayerController>(GetController());
	GOOSE_BAIL(PC);

	World->DestroyActor(this);

	TActorIterator<APlayerStart> It(World);
	if (It)
	{
		FTransform Transform = (*It)->GetActorTransform();

		AGroundHogBall* Ball = World->SpawnActor<AGroundHogBall>(AGroundHogBall::StaticClass(), Transform);
		if (GOOSE_VERIFY(Ball))
		{
			PC->bAutoManageActiveCameraTarget = false;
			PC->Possess(Ball);

			FViewTargetTransitionParams TransParams;
			TransParams.BlendExp = 3;
			TransParams.BlendFunction = EViewTargetBlendFunction::VTBlend_EaseInOut;
			TransParams.BlendTime = 1.0f;
			PC->SetViewTarget(Ball, TransParams);
		}
	}
}

void AGroundHogBall::QuitGame()
{
	UKismetSystemLibrary::QuitGame(this, Cast<APlayerController>(GetController()), EQuitPreference::Quit, false);
}

void AGroundHogBall::UpdateBlinking(float DeltaSeconds)
{
	BlinkTimer -= DeltaSeconds;

	if (BlinkTimer <= 0.f)
	{
		SetBlinking(!bBlinking);

		if (bBlinking)
		{
			BlinkTimer = HogConsts::BLINK_TIME;
		}
		else
		{
			BlinkTimer = FMath::FRandRange(HogConsts::TIME_BETWEEN_BLINK_MIN, HogConsts::TIME_BETWEEN_BLINK_MAX);
		}
	}
}

FQuat AGroundHogBall::GetUpQuaternion() const
{
	// Find quaternion that adjusts the up vector towards "up" (i.e. opposite of gravity)
	return FQuat::FindBetweenVectors(FVector::UpVector, -GravityDirection);
}
