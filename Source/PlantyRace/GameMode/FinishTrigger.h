#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FinishTrigger.generated.h"

class USceneComponent;
class UBoxComponent;

UCLASS()
class PLANTYRACE_API AFinishTrigger : public AActor
{
	GENERATED_BODY()

public:
	AFinishTrigger();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBoxComponent> TriggerBox;

	UFUNCTION()
	void OnFinishTriggerBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);
};