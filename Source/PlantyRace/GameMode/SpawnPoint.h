#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpawnPoint.generated.h"

class USceneComponent;
class UArrowComponent;

UCLASS()
class PLANTYRACE_API ASpawnPoint : public AActor
{
	GENERATED_BODY()

public:
	ASpawnPoint();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UArrowComponent> ArrowComponent;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SpawnPoint")
	int32 SpawnIndex = 0;

	FORCEINLINE UArrowComponent* GetArrowComponent() const { return ArrowComponent; }
};