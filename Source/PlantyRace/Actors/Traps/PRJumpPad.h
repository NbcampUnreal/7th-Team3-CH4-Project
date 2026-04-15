#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PRJumpPad.generated.h"

class UStaticMeshComponent;
class UBoxComponent;
class APlantyRaceCharacter;

UENUM(BlueprintType)
enum class EJumpPadMode : uint8
{
	Z,
	XZ
};

UCLASS()
class PLANTYRACE_API APRJumpPad : public AActor
{
	GENERATED_BODY()
	
public:	
	APRJumpPad();

protected:
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* MeshComp;

	UPROPERTY(VisibleAnywhere)
	UBoxComponent* BoxComp;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Jump")
	EJumpPadMode JumpMode = EJumpPadMode::XZ;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Jump")
	float LaunchUpPower = 1200.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Jump")
	float LaunchForwardPower = 600.f;

	UFUNCTION()
	void OnBeginOverlap(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	FVector GetLaunchVelocity() const;
};
