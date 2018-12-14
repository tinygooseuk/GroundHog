// Copyright 2018 TinyGoose Ltd., All Rights Reserved.

#include "GroundHogGameMode.h"
#include "GroundHogBall.h"

#include "Blueprint/UserWidget.h"
#include "ConstructorHelpers.h"

AGroundHogGameMode::AGroundHogGameMode()
{
	// set default pawn class to our ball
	DefaultPawnClass = AGroundHogBall::StaticClass();

	UMG_GemCounter = FIND_CLASS(UMG_GemCounter, "UMG");
}

void AGroundHogGameMode::BeginPlay() 
{
	Super::BeginPlay();

	// Create gem counter & add to screen
	UUserWidget* GemCounter = CreateWidget<UUserWidget>(GetWorld()->GetFirstPlayerController(), UMG_GemCounter);
	if (GOOSE_VERIFY(GemCounter))
	{
		GemCounter->AddToViewport();
	}
}
