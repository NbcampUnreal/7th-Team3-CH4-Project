#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RespawnTrigger.generated.h"

class UBoxComponent;

UCLASS()
class PLANTYRACE_API ARespawnTrigger : public AActor
{
	GENERATED_BODY()

public:
	ARespawnTrigger();

protected:

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UBoxComponent> TriggerBox;

	virtual void BeginPlay() override;

	UFUNCTION()
	void OnOverlapBegin(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);
};