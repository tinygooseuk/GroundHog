// Copyright 2018 TinyGoose Ltd., All Rights Reserved.

#pragma once

#include "GroundHog.h"
#include "GameFramework/GameModeBase.h"
#include "GroundHogGameMode.generated.h"

UCLASS(minimalapi)
class AGroundHogGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AGroundHogGameMode();

	virtual void BeginPlay() override;

private:
	class UClass* UMG_GemCounter;
};



