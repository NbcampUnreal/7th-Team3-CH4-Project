#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PRTornadoComponent.generated.h"

class APlantyRaceCharacter;
class AWeatherEffectZone;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PLANTYRACE_API UPRTornadoComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UPRTornadoComponent();

protected:
	UPROPERTY()
	TObjectPtr<APlantyRaceCharacter> OwnerCharacter = nullptr;

	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(ReplicatedUsing = OnRep_InTornado)
	bool bInTornado = false;
		
	UPROPERTY(VisibleAnywhere, Category = "Tornado")
	float TornadoElapsedTime = 0.0f;

	UPROPERTY(VisibleAnywhere, Category = "Tornado")
	float TornadoTotalDuration = 2.5f;

	UPROPERTY(VisibleAnywhere, Category = "Tornado")
	float TornadoRiseDuration = 0.7f;

	UPROPERTY(EditAnywhere, Category = "Tornado")
	float TornadoRiseSpeed = 500.0f;

	UPROPERTY(EditAnywhere, Category = "Tornado")
	float TornadoFallSpeed = -250.0f;

	UPROPERTY(EditAnywhere, Category = "Tornado")
	float TornadoSuctionSpeed = 400.0f;

	UPROPERTY(EditAnywhere, Category = "Tornado")
	float TornadoOrbitSpeed = 250.0f;

	UPROPERTY(EditAnywhere, Category = "Tornado")
	float TornadoMinSuctionDistance = 100.f;

	UPROPERTY(EditAnywhere, Category = "Tornado")
	float TornadoMaxSuctionDistance = 650.f;

	UPROPERTY(EditAnywhere, Category = "Tornado")
	float TornadoMinSuctionScale = 0.35f;

	UPROPERTY(EditAnywhere, Category = "Tornado")
	float TornadoMaxSuctionScale = 1.0f;

	UPROPERTY(EditAnywhere, Category = "Tornado")
	float TornadoSuctionEaseExponent = 2.0f;

	UPROPERTY(Replicated)
	TObjectPtr<AActor> TornadoSourceActor = nullptr;

	UPROPERTY(Replicated)
	TObjectPtr<AWeatherEffectZone> CurrentTornadoZone = nullptr;

	FTimerHandle TornadoTimerHandle;

public:
	void EnterTornado(AActor* InTornadoSource);

	void ExitTornado();

	bool IsInTornado() const { return bInTornado; }

	bool IsRisePhase() const;

	bool IsTornadoFinished() const;

	FVector GetSuctionVelocity(const FVector& ToCenter) const;

	FVector GetOrbitVelocity(const FVector& ToCenter) const;

	FVector GetVerticalVelocity() const;

	AWeatherEffectZone* GetCurrentTornadoZone() const { return CurrentTornadoZone; }

	void UpdateTornadoMovement(float DeltaTime);

	UFUNCTION()
	void OnRep_InTornado();
};
