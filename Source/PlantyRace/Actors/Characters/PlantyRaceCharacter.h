// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "Types/WeatherEffectTypes.h"
#include "PlantyRaceCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputAction;
struct FInputActionValue;
class UCharacterEffectComponent;
class APRGameStateBase;
class AWeatherEffectZone;

UENUM(BlueprintType)
enum class EFootType : uint8
{
	Left,
	Right
};

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

/**
 *  A simple player-controllable third person character
 *  Implements a controllable orbiting camera
 */
UCLASS(abstract)
class APlantyRaceCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	APlantyRaceCharacter();
	
	void Look(const FInputActionValue& Value);
	void Move(const FInputActionValue& Value);
	void StartJump(const FInputActionValue& Value);
	void EndJump(const FInputActionValue& Value);
	void Grab(const FInputActionValue& Value);

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	TObjectPtr<class UInputAction> MoveAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	TObjectPtr<UInputAction> LookAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	TObjectPtr<UInputAction> JumpAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	TObjectPtr<UInputAction> GrabAction;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	TObjectPtr<UInputAction> IA_RandomizeClothes;

	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Camera")
	TObjectPtr<class USpringArmComponent> SpringArmComp;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Camera")
	TObjectPtr<class UCameraComponent> CameraComp;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Camera")
	float MouseSensitivity;
	
public:
	
	UPROPERTY()
	TArray<TObjectPtr<USkeletalMeshComponent>> ModularMeshes;
	
	UPROPERTY(EditAnywhere, Category="Customization")
	TArray<TObjectPtr<USkeletalMesh>> PantsOptions;

	UPROPERTY(EditAnywhere, Category="Customization")
	TArray<TObjectPtr<USkeletalMesh>> ShirtOptions;

	UPROPERTY(EditAnywhere, Category="Customization")
	TArray<TObjectPtr<USkeletalMesh>> HairOptions;
	
	UPROPERTY(EditAnywhere, Category="Customization")
	TArray<TObjectPtr<USkeletalMesh>> GlassOptions;
	
	UPROPERTY(EditAnywhere, Category="Customization")
	TArray<TObjectPtr<USkeletalMesh>> ShoeOptions;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="ClothMath")
	USkeletalMeshComponent* PantsSkeletalMesh;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="ClothMath")
	USkeletalMeshComponent* ShirtSkeletalMesh;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="ClothMath")
	USkeletalMeshComponent* HairSkeletalMesh;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="ClothMath")
	USkeletalMeshComponent* GlassSkeletalMesh;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="ClothMath")
	USkeletalMeshComponent* ShoeSkeletalMesh;
	
	
public:
	UFUNCTION(Server, Reliable)
	void ServerGrab();

	UFUNCTION(Server, Reliable)
	void ServerRelease();
	
	UFUNCTION()
	void RandomizeClothes();
	void InitializeModularMeshes();
	void SetRandomMesh(USkeletalMeshComponent* TargetMesh, const TArray<TObjectPtr<USkeletalMesh>>& Options);
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	
	UPROPERTY(Replicated)
	bool bIsGrabbed = false;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual bool CanJumpInternal_Implementation() const override;

	void EnterWeatherZone(
		ESurfaceEffectType InSurfaceEffectType,
		float InMoveSpeedMultiplier,
		float InJumpMultiplier,
		bool bInBlockJump
	);

	void ExitWeatherZone();

	void EnterTornado(AActor* InTornadoSource);

	void ExitTornado();

	UFUNCTION(BlueprintCallable)
	void PlayFootstepSound(EFootType FootType);

	float GetDefaultMaxWalkSpeed() const { return DefaultMaxWalkSpeed; }

	float GetDefaultJumpZVelocity() const { return DefaultJumpZVelocity; }

	AWeatherEffectZone* GetCurrentTornadoZone() const { return CurrentTornadoZone; }

protected:
	UPROPERTY(VisibleAnywhere, Category = "Zone")
	float DefaultMaxWalkSpeed = 0.f;

	UPROPERTY(VisibleAnywhere, Category = "Zone")
	float DefaultJumpZVelocity = 0.f;

	UPROPERTY(EditAnywhere, Category = "Audio")
	TObjectPtr<class USoundBase> PuddleFootstepSound;

	UPROPERTY(EditAnywhere, Category = "Audio")
	FName RightFootstepSocketName;

	UPROPERTY(EditAnywhere, Category = "Audio")
	FName LeftFootstepSocketName;

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

	void UpdateTornadoMovement(float DeltaTime);

	bool IsRisePhase() const;

	bool IsTornadoFinished() const;

	FVector GetSuctionVelocity(const FVector& ToCenter) const;

	FVector GetOrbitVelocity(const FVector& ToCenter) const;

	FVector GetVerticalVelocity() const;

	UPROPERTY(VisibleAnywhere)
	UCharacterEffectComponent* CharacterEffectComp;

	UFUNCTION()
	void OnRep_InTornado();

	UFUNCTION()
	void HandleWeatherChanged();
};
