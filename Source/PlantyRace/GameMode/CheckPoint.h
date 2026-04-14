#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ArrowComponent.h"
#include "CheckPoint.generated.h"

class UBoxComponent;
class UArrowComponent;
class USceneComponent;

UCLASS()
class PLANTYRACE_API ACheckPoint : public AActor
{
	GENERATED_BODY()

public:
	ACheckPoint();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBoxComponent> BoxCollision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UArrowComponent> ArrowComponent;

	UFUNCTION()
	void OnCheckpointBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
	int32 CheckpointIndex = 0;

	FORCEINLINE UBoxComponent* GetBoxCollision() const { return BoxCollision; }
	FORCEINLINE UArrowComponent* GetArrowComponent() const { return ArrowComponent; }

	FORCEINLINE FVector GetRespawnLocation() const { return ArrowComponent ? ArrowComponent->GetComponentLocation() : GetActorLocation(); }
	FORCEINLINE FRotator GetRespawnRotation() const { return ArrowComponent ? ArrowComponent->GetComponentRotation() : GetActorRotation(); }
};