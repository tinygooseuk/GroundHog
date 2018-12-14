// Copyright 2018 TinyGoose Ltd., All Rights Reserved.

#include "Gem.h"

// Engine
#include "Blueprint/UserWidget.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/CollisionProfile.h"
#include "Engine/StaticMesh.h"
#include "EngineUtils.h"
#include "GameFramework/RotatingMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"
#include "Sound/SoundWave.h"

// Game
#include "GroundHogBall.h"

AGem::AGem()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh> GemMeshResource(TEXT("/Game/Meshes/SM_Gem.SM_Gem"));

	// Create mesh component for the gem
	GemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Gem0"));
	GemMesh->SetStaticMesh(GemMeshResource.Object);
	GemMesh->SetCollisionProfileName(TEXT("OverlapAll"));
	GemMesh->SetGenerateOverlapEvents(true);
	RootComponent = GemMesh;

	// Create spinning effect
	Rotator = CreateDefaultSubobject<URotatingMovementComponent>(TEXT("Rotator0"));
	Rotator->UpdatedComponent = GemMesh;
	Rotator->RotationRate = FRotator(0.f, 100.f, 0.f);

	// Get pickup sound
	PickupSound = FIND_RESOURCE(SoundWave, Pickup, "Sounds");

	// Get end UI 
	UMG_YouWin = FIND_CLASS(UMG_YouWin, "UMG");
}

void AGem::BeginPlay()
{
	Super::BeginPlay();
	
}

void AGem::NotifyActorBeginOverlap(class AActor* Other)
{
	Super::NotifyActorBeginOverlap(Other);

	if (Cast<AGroundHogBall>(Other))
	{
		// Check if last one
		TArray<AActor*> GemsLeft;
		UGameplayStatics::GetAllActorsOfClass(this, AGem::StaticClass(), GemsLeft);

		if (GemsLeft.Num() == 1)
		{
			// Add end-game screen
			UUserWidget* YouWin = CreateWidget<UUserWidget>(GetWorld()->GetFirstPlayerController(), UMG_YouWin);
			if (GOOSE_VERIFY(YouWin))
			{
				YouWin->AddToViewport();
			}
		}

		// Play sound & remove
		UGameplayStatics::PlaySoundAtLocation(this, PickupSound, GetActorLocation());
		Destroy();
	}
}
