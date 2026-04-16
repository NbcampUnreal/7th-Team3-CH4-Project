#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PRTrapSpawnVolume.generated.h"

class UBoxComponent;
class APRRollingActor;

UCLASS()
class PLANTYRACE_API APRTrapSpawnVolume : public AActor
{
	GENERATED_BODY()
	
public:	
	APRTrapSpawnVolume();

protected:
	UPROPERTY(VisibleAnywhere)
	UBoxComponent* BoxComp;

	UPROPERTY(EditAnywhere, Category = "Spawn")
	TSubclassOf<APRRollingActor> RollingActorClass;

	UPROPERTY(EditAnywhere, Category = "Spawn")
	float SpawnInterval = 3.f;

	UPROPERTY(EditAnywhere, Category = "Spawn")
	FVector BoxExtent = FVector(80.f, 80.f, 60.f);

	virtual void BeginPlay() override;

	FVector GetRandomPointVolume() const;
	void SpawnRollingActor();

	FTimerHandle SpawnTimerHandle;
};
