// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "PlantyRaceCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputAction;
struct FInputActionValue;

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

};
