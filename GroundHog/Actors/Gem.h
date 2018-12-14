// Copyright 2018 TinyGoose Ltd., All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Gem.generated.h"

UCLASS()
class GROUNDHOG_API AGem : public AActor
{
	GENERATED_BODY()
	
public:	
	AGem();
	virtual void BeginPlay() override;
	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

private:
	///////////////////////// Components /////////////////////////
	/** StaticMesh used for the gem */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Gem, meta = (AllowPrivateAccess = "true"))
	class UStaticMeshComponent* GemMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Gem, meta = (AllowPrivateAccess = "true"))
	class URotatingMovementComponent* Rotator;

	UPROPERTY()
	class USoundWave* PickupSound;

	UPROPERTY()
	class UClass* UMG_YouWin;
};
