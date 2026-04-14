#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerStart.h"
#include "SpawnPoint.generated.h"

UCLASS()
class PLANTYRACE_API ASpawnPoint : public APlayerStart
{
	GENERATED_BODY()

public:
	ASpawnPoint(const FObjectInitializer& ObjectInitializer);

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpawnPoint")
	int32 SpawnIndex = 0;
};