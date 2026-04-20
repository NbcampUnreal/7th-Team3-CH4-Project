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
class UPRKnockbackComponent;
class UPRTornadoComponent;
class UAnimMontage;

UENUM(BlueprintType)
enum class EFootType : uint8
{
	Left,
	Right
};

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UENUM(BlueprintType)
enum class EPlayerActionState : uint8
{
	Idle		UMETA(DisplayName="Idle"),
	Jump		UMETA(DisplayName="Jump"),
	Dive		UMETA(DisplayName="Dive"),
	Slide		UMETA(DisplayName="Slide"),
	Attack		UMETA(DisplayName="Attack"),
	KnockedDown UMETA(DisplayName = "KnockedDown")
};

USTRUCT(BlueprintType)
struct FClothesRepData
{
	GENERATED_BODY()

	UPROPERTY()
	int32 PantsIndex = -1;

	UPROPERTY()
	int32 ShirtIndex = -1;

	UPROPERTY()
	int32 HairIndex = -1;

	UPROPERTY()
	int32 GlassIndex = -1;

	UPROPERTY()
	int32 ShoeIndex = -1;
};


UCLASS(abstract)
class APlantyRaceCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	APlantyRaceCharacter(const FObjectInitializer& ObjectInitializer);

	void Look(const FInputActionValue& Value);
	void Move(const FInputActionValue& Value);
	void StartJump(const FInputActionValue& Value);
	void EndJump(const FInputActionValue& Value);
	void StartGrab(const FInputActionValue& Value);
	void EndGrab(const FInputActionValue& Value);
	void Dive(const FInputActionValue& Value);
	void Landed(const FHitResult& Hit);

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<class UInputAction> MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> GrabAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> DiveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_RandomizeClothes;


public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grab")
	TObjectPtr<USceneComponent> GrabHoldPoint;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<class USpringArmComponent> SpringArmComp;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<class UCameraComponent> CameraComp;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mouse")
	float MouseSensitivity;

	UPROPERTY(EditAnywhere, Category = "Dive")
	float DiveForwardStrength = 1100.f;

	UPROPERTY(EditAnywhere, Category = "Dive")
	float DiveUpStrength = 120.f;


	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|Slope")
	float BaseWalkSpeed = 600.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|Slope")
	TObjectPtr<UCurveFloat> UphillSpeedCurve;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement|Slope")
	TObjectPtr<UCurveFloat> DownhillSpeedCurve;
public:
	UPROPERTY()
	TArray<TObjectPtr<USkeletalMeshComponent>> ModularMeshes;

	UPROPERTY(EditAnywhere, Category = "Customization")
	TArray<TObjectPtr<USkeletalMesh>> PantsOptions;

	UPROPERTY(EditAnywhere, Category = "Customization")
	TArray<TObjectPtr<USkeletalMesh>> ShirtOptions;

	UPROPERTY(EditAnywhere, Category = "Customization")
	TArray<TObjectPtr<USkeletalMesh>> HairOptions;

	UPROPERTY(EditAnywhere, Category = "Customization")
	TArray<TObjectPtr<USkeletalMesh>> GlassOptions;

	UPROPERTY(EditAnywhere, Category = "Customization")
	TArray<TObjectPtr<USkeletalMesh>> ShoeOptions;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ClothMath")
	USkeletalMeshComponent* PantsSkeletalMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ClothMath")
	USkeletalMeshComponent* ShirtSkeletalMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ClothMath")
	USkeletalMeshComponent* HairSkeletalMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ClothMath")
	USkeletalMeshComponent* GlassSkeletalMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ClothMath")
	USkeletalMeshComponent* ShoeSkeletalMesh;

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Montage")
	TObjectPtr<class UAnimMontage> DiveMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Montage")
	TObjectPtr<class UAnimMontage> GrabMontage;

public:
	UFUNCTION(Server, Reliable)
	void ServerGrab();

	UFUNCTION(Server, Reliable)
	void ServerRelease();

	UFUNCTION(Server, Reliable)
	void ServerRandomizeClothes();
protected:
	UFUNCTION()
	void OnRep_ClothesData();
	
	void ApplyClothesFromRepData();

	int32 GetRandomValidIndex(const TArray<TObjectPtr<USkeletalMesh>>& Options) const;

	UPROPERTY(ReplicatedUsing = OnRep_ClothesData)
	FClothesRepData ClothesData;
	void SetMeshByIndex(USkeletalMeshComponent* TargetMesh, const TArray<TObjectPtr<USkeletalMesh>>& Options, int32 Index);

public:
	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_PlayGrabSound();
	
	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_PlayDivingSound();
	
	UFUNCTION()
	void PlayFootstepSounds(EFootType FootType);
	void PlayJumpSounds();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sound")
	TObjectPtr<class USoundBase> GrabSound;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sound")
	TObjectPtr<class USoundBase> DiveSound;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sound")
	TObjectPtr<class USoundBase> RightFootstepSound;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sound")
	TObjectPtr<class USoundBase>LeftFootstepSound;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Sound")
	TObjectPtr<class USoundBase>JumpSound;
	
public:
	UFUNCTION()
	void RandomizeClothes();
	void InitializeModularMeshes();
	void SetRandomMesh(USkeletalMeshComponent* TargetMesh, const TArray<TObjectPtr<USkeletalMesh>>& Options);
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	float GetFloorSlopeAngle() const;
	float GetSlopeMoveDirectionDot() const;


	UPROPERTY(Replicated)
	bool bIsGrabbed = false;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	EPlayerActionState CurrentActionState = EPlayerActionState::Idle;

	void UpdateSlopeSpeed();
	bool CanMove() const;
	bool CanJumpAction() const;
	bool CanGrabAction() const;
	bool CanDiveAction() const;
	void SetActionState(EPlayerActionState NewState);
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

	void SetKnockedDown(bool bValue);

	float GetDefaultMaxWalkSpeed() const { return DefaultMaxWalkSpeed; }

	float GetDefaultJumpZVelocity() const { return DefaultJumpZVelocity; }

	AWeatherEffectZone* GetCurrentTornadoZone() const;

	bool IsKnockedDown() const;

	UPRKnockbackComponent* GetKnockbackComp() const { return KnockbackComp; }

	void PlayKnockedDownMontage();

	void PlayGetUpMontage();

	UFUNCTION()
	void LockMovement();

	UFUNCTION()
	void UnlockMovement();

protected:
	UPROPERTY(Replicated, EditAnywhere, Category = "Pet")
	TArray<TSubclassOf<class APRPetCharacter>> PetClasses;

	// 현재 펫
	UPROPERTY(Replicated, VisibleAnywhere, Category = "Pet")
	TObjectPtr<APRPetCharacter> CurrentPet;

	// 입력
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> ChangePetAction;
	
protected:
	
	void ChangePetInput();
	UFUNCTION(Server, Reliable)
	void ServerChangePet();
	

	
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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grab")
	float GrabRange = 200.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grab")
	float GrabRadius = 50.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Montage")
	TObjectPtr<UAnimMontage> KnockedDownMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Montage")
	TObjectPtr<UAnimMontage> GetUpMontage;

	UPROPERTY(Replicated)
	TObjectPtr<APlantyRaceCharacter> GrabTarget = nullptr;

	UPROPERTY(Replicated)
	TObjectPtr<APlantyRaceCharacter> GrabbedBy = nullptr;

	UPROPERTY(ReplicatedUsing = OnRep_IsKnockedDown)
	bool bIsKnockedDown = false;

	UPROPERTY(VisibleAnywhere)
	UCharacterEffectComponent* CharacterEffectComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPRKnockbackComponent> KnockbackComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UPRTornadoComponent> TornadoComp;

	UFUNCTION()
	void OnRep_IsKnockedDown();

	UFUNCTION()
	void HandleWeatherChanged();

	void HandleKnockedDownChanged();

public:
	// 리스폰 관련 추가
	UFUNCTION(BlueprintCallable, Category = "Respawn")
	void SetLastCheckpoint(class ACheckPoint* NewCheckpoint, int32 NewCheckpointIndex);

	UFUNCTION(BlueprintCallable, Category = "Respawn")
	class ACheckPoint* GetLastCheckpoint() const { return LastCheckpoint; }

	UFUNCTION(BlueprintCallable, Category = "Respawn")
	int32 GetLastCheckpointIndex() const { return LastCheckpointIndex; }

	UFUNCTION(BlueprintCallable, Category = "Respawn")
	void SetStartSpawnPoint(class ASpawnPoint* NewSpawnPoint);

	UFUNCTION(BlueprintCallable, Category = "Respawn")
	class ASpawnPoint* GetStartSpawnPoint() const { return StartSpawnPoint; }

protected:
	// 리스폰 관련 추가
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Respawn")
	TObjectPtr<class ACheckPoint> LastCheckpoint = nullptr;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Respawn")
	int32 LastCheckpointIndex = -1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Respawn")
	TObjectPtr<class ASpawnPoint> StartSpawnPoint = nullptr;
};