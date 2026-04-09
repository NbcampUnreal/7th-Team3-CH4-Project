#include "PlantyRaceCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Net/UnrealNetwork.h"
#include "Curves/CurveFloat.h"
#include "PRCharacterMovementComponent.h"

APlantyRaceCharacter::APlantyRaceCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(
	ObjectInitializer.SetDefaultSubobjectClass<
	UPRCharacterMovementComponent>(
	ACharacter::CharacterMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;
	GetCharacterMovement()->JumpZVelocity = 700.f;
	
	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
	SpringArmComp->SetupAttachment(RootComponent);
	SpringArmComp->TargetArmLength = 800.f;
	SpringArmComp->SetRelativeRotation(FRotator(-55.f, 0.f, 0.f)); // 위에서 아래로
	SpringArmComp->bUsePawnControlRotation = true;
	
	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	CameraComp->SetupAttachment(SpringArmComp, USpringArmComponent::SocketName);
	CameraComp->bUsePawnControlRotation = false;
	
	PantsSkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("PantsMesh"));
	PantsSkeletalMesh->SetupAttachment(GetMesh());
	PantsSkeletalMesh->SetLeaderPoseComponent(GetMesh());

	ShirtSkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ShirtMesh"));
	ShirtSkeletalMesh->SetupAttachment(GetMesh());
	ShirtSkeletalMesh->SetLeaderPoseComponent(GetMesh());

	HairSkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("HairMesh"));
	HairSkeletalMesh->SetupAttachment(GetMesh());
	HairSkeletalMesh->SetLeaderPoseComponent(GetMesh());
	
	GlassSkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("GlassMesh"));
	GlassSkeletalMesh->SetupAttachment(GetMesh());
	GlassSkeletalMesh->SetLeaderPoseComponent(GetMesh());
	
	ShoeSkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ShoeMesh"));
	ShoeSkeletalMesh->SetupAttachment(GetMesh());
	ShoeSkeletalMesh->SetLeaderPoseComponent(GetMesh());
	
	
	MouseSensitivity =1.5f;
	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}




void APlantyRaceCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D LookValue = Value.Get<FVector2D>();


	if (!Controller)
	{
		return;
	}

	AddControllerYawInput(LookValue.X * MouseSensitivity);
	AddControllerPitchInput(-LookValue.Y * MouseSensitivity);
}

void APlantyRaceCharacter::Move(const FInputActionValue& Value)
{
	if (!CanMove())
	{
		return;
	}
	const FVector2D MoveValue = Value.Get<FVector2D>();

	if (!Controller)
	{
		return;
	}

	const FRotator ControlRot = Controller->GetControlRotation();
	const FRotator YawRot(0.f, ControlRot.Yaw, 0.f);

	const FVector ForwardDir = FRotationMatrix(YawRot).GetUnitAxis(EAxis::X);
	const FVector RightDir = FRotationMatrix(YawRot).GetUnitAxis(EAxis::Y);

	AddMovementInput(ForwardDir, MoveValue.Y);
	AddMovementInput(RightDir, MoveValue.X);
}

void APlantyRaceCharacter::StartJump(const FInputActionValue& Value)
{
	Jump();
}

void APlantyRaceCharacter::EndJump(const FInputActionValue& Value)
{
	StopJumping();
}

void APlantyRaceCharacter::Grab(const FInputActionValue& Value)
{
	ServerGrab();
}

void APlantyRaceCharacter::Dive(const FInputActionValue& Value)
{
	if (!CanDiveAction())
	{
		return;
	}

	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	if (!MoveComp)
	{
		return;
	}

	SetActionState(EPlayerActionState::Dive);

	FVector HorizontalVelocity = GetVelocity();
	HorizontalVelocity.Z = 0.f;

	FVector DiveDirection = GetActorForwardVector();
	DiveDirection.Z = 0.f;
	DiveDirection.Normalize();

	const float DiveBoost = 400.f;
	const float MaxDiveSpeed = 700.f;

	const float SpeedAlongDive = FVector::DotProduct(HorizontalVelocity, DiveDirection);

	const float FinalSpeedAlongDive =
		FMath::Clamp(SpeedAlongDive + DiveBoost, 0.f, MaxDiveSpeed);

	FVector NewVelocity = DiveDirection * FinalSpeedAlongDive;
	float CurrentZ = GetVelocity().Z;
	NewVelocity.Z = CurrentZ + 170.f;

	LaunchCharacter(NewVelocity, true, true);
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance)
	{
		if (DiveMontage)
		AnimInstance->Montage_Play(DiveMontage);
	}
}

void APlantyRaceCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);

	SetActionState(EPlayerActionState::Idle);
}

void APlantyRaceCharacter::ServerGrab_Implementation()
{
	FVector Start = GetActorLocation();
	FVector End = Start + GetActorForwardVector() * 200.f;

	FHitResult Hit;
	GetWorld()->SweepSingleByChannel(
		Hit,
		Start,
		End,
		FQuat::Identity,
		ECC_Pawn,
		FCollisionShape::MakeSphere(50.f)
	);

	APlantyRaceCharacter* Target = Cast<APlantyRaceCharacter>(Hit.GetActor());
	if (Target)
	{
		//잡은 타켓이 어떻게 될지는 아직 미정.
	}

}

void APlantyRaceCharacter::ServerRelease_Implementation()
{
	
}

// Called when the game starts or when spawned
void APlantyRaceCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	InitializeModularMeshes();
	ApplyClothesFromRepData();
}

// Called every frame
void APlantyRaceCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APlantyRaceCharacter::ServerRandomizeClothes_Implementation()
{
	ClothesData.PantsIndex = GetRandomValidIndex(PantsOptions);
	ClothesData.ShirtIndex = GetRandomValidIndex(ShirtOptions);
	ClothesData.HairIndex = GetRandomValidIndex(HairOptions);
	ClothesData.GlassIndex = GetRandomValidIndex(GlassOptions);
	ClothesData.ShoeIndex = GetRandomValidIndex(ShoeOptions);
	
	ApplyClothesFromRepData();
	ForceNetUpdate();
}

void APlantyRaceCharacter::OnRep_ClothesData()
{
	ApplyClothesFromRepData();
}

void APlantyRaceCharacter::ApplyClothesFromRepData()
{
	SetMeshByIndex(PantsSkeletalMesh, PantsOptions, ClothesData.PantsIndex);
	SetMeshByIndex(ShirtSkeletalMesh, ShirtOptions, ClothesData.ShirtIndex);
	SetMeshByIndex(HairSkeletalMesh, HairOptions, ClothesData.HairIndex);
	SetMeshByIndex(GlassSkeletalMesh, GlassOptions, ClothesData.GlassIndex);
	SetMeshByIndex(ShoeSkeletalMesh, ShoeOptions, ClothesData.ShoeIndex);
}

int32 APlantyRaceCharacter::GetRandomValidIndex(const TArray<TObjectPtr<USkeletalMesh>>& Options) const
{
	if (Options.Num() <= 0)
	{
		return -1;
	}

	return FMath::RandRange(0, Options.Num() - 1);
}

void APlantyRaceCharacter::SetMeshByIndex(USkeletalMeshComponent* TargetMesh,
	const TArray<TObjectPtr<USkeletalMesh>>& Options, int32 Index)
{
	if (!TargetMesh)
	{
		return;
	}

	if (!Options.IsValidIndex(Index))
	{
		TargetMesh->SetSkeletalMesh(nullptr);
		return;
	}

	TargetMesh->SetSkeletalMesh(Options[Index]);
	TargetMesh->SetLeaderPoseComponent(GetMesh());
}

void APlantyRaceCharacter::RandomizeClothes()
{
	ServerRandomizeClothes();
}

void APlantyRaceCharacter::InitializeModularMeshes()
{
	ModularMeshes.Empty();

	if (PantsSkeletalMesh)
	{
		PantsSkeletalMesh->SetLeaderPoseComponent(GetMesh());
		ModularMeshes.Add(PantsSkeletalMesh);
	}

	if (ShirtSkeletalMesh)
	{
		ShirtSkeletalMesh->SetLeaderPoseComponent(GetMesh());
		ModularMeshes.Add(ShirtSkeletalMesh);
	}

	if (HairSkeletalMesh)
	{
		HairSkeletalMesh->SetLeaderPoseComponent(GetMesh());
		ModularMeshes.Add(HairSkeletalMesh);
	}

	if (GlassSkeletalMesh)
	{
		GlassSkeletalMesh->SetLeaderPoseComponent(GetMesh());
		ModularMeshes.Add(GlassSkeletalMesh);
	}
	
	if (ShoeSkeletalMesh)
	{
		ShoeSkeletalMesh->SetLeaderPoseComponent(GetMesh());
		ModularMeshes.Add(ShoeSkeletalMesh);
	}
}

void APlantyRaceCharacter::SetRandomMesh(USkeletalMeshComponent* TargetMesh, const TArray<TObjectPtr<USkeletalMesh>>& Options)
{
	if (!TargetMesh || Options.Num() == 0)
	{
		return;
	}

	const int32 RandomIndex = FMath::RandRange(0, Options.Num() - 1);
	TargetMesh->SetSkeletalMesh(Options[RandomIndex]);
}

void APlantyRaceCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APlantyRaceCharacter, bIsGrabbed);
	DOREPLIFETIME(APlantyRaceCharacter, ClothesData);
}

float APlantyRaceCharacter::GetFloorSlopeAngle() const
{
	if (!GetCharacterMovement())
	{
		return 0.f;
	}

	const FFindFloorResult& FloorResult = GetCharacterMovement()->CurrentFloor;

	if (!FloorResult.IsWalkableFloor())
	{
		return 0.f;
	}

	const FVector FloorNormal = FloorResult.HitResult.ImpactNormal;
	const float Dot = FVector::DotProduct(FloorNormal, FVector::UpVector);
	const float ClampedDot = FMath::Clamp(Dot, -1.f, 1.f);

	return FMath::RadiansToDegrees(FMath::Acos(ClampedDot));
}

float APlantyRaceCharacter::GetSlopeMoveDirectionDot() const
{
	if (!GetCharacterMovement())
	{
		return 0.f;
	}

	const FFindFloorResult& FloorResult = GetCharacterMovement()->CurrentFloor;
	if (!FloorResult.IsWalkableFloor())
	{
		return 0.f;
	}

	const FVector FloorNormal = FloorResult.HitResult.ImpactNormal;

	FVector Velocity2D = GetVelocity();
	Velocity2D.Z = 0.f;

	if (Velocity2D.IsNearlyZero())
	{
		return 0.f;
	}

	Velocity2D.Normalize();

	FVector UphillDirection = FVector::VectorPlaneProject(FVector::UpVector, FloorNormal).GetSafeNormal();

	return FVector::DotProduct(Velocity2D, UphillDirection);
}

void APlantyRaceCharacter::UpdateSlopeSpeed()
{
	UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	if (!MoveComp)
	{
		return;
	}

	float FinalSpeed = BaseWalkSpeed;

	const float SlopeAngle = GetFloorSlopeAngle();
	const float MoveDot = GetSlopeMoveDirectionDot();

	if (MoveDot > 0.1f)
	{
		if (UphillSpeedCurve)
		{
			const float Multiplier = UphillSpeedCurve->GetFloatValue(SlopeAngle);
			FinalSpeed = BaseWalkSpeed * Multiplier;
		}
	}
	else if (MoveDot < -0.1f)
	{
		if (DownhillSpeedCurve)
		{
			const float Multiplier = DownhillSpeedCurve->GetFloatValue(SlopeAngle);
			FinalSpeed = BaseWalkSpeed * Multiplier;
		}
	}
	else
	{
		FinalSpeed = BaseWalkSpeed;
	}

	MoveComp->MaxWalkSpeed = FinalSpeed;
}


bool APlantyRaceCharacter::CanMove() const
{
	return CurrentActionState != EPlayerActionState::Dive
		&& CurrentActionState != EPlayerActionState::Attack
		&& CurrentActionState != EPlayerActionState::Slide;
}

bool APlantyRaceCharacter::CanJumpAction() const
{
	return CurrentActionState == EPlayerActionState::Idle;
}

bool APlantyRaceCharacter::CanGrabAction() const
{
	return CurrentActionState == EPlayerActionState::Idle
		|| CurrentActionState == EPlayerActionState::Jump;
}

bool APlantyRaceCharacter::CanDiveAction() const
{
	const UCharacterMovementComponent* MoveComp = GetCharacterMovement();
	if (!MoveComp)
	{
		return false;
	}

	return MoveComp->IsFalling()
		&& CurrentActionState != EPlayerActionState::Dive
		&& CurrentActionState != EPlayerActionState::Attack
		&& CurrentActionState != EPlayerActionState::Slide;
}

void APlantyRaceCharacter::SetActionState(EPlayerActionState NewState)
{
	CurrentActionState = NewState;
}

// Called to bind functionality to input
void APlantyRaceCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInput =Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		
		if (MoveAction)
		{
			EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APlantyRaceCharacter::Move);
		}

		if (LookAction)
		{
			EnhancedInput->BindAction(LookAction, ETriggerEvent::Triggered, this, &APlantyRaceCharacter::Look);
		}

		if (JumpAction)
		{
			EnhancedInput->BindAction(JumpAction, ETriggerEvent::Started, this, &APlantyRaceCharacter::StartJump);
			EnhancedInput->BindAction(JumpAction, ETriggerEvent::Completed, this, &APlantyRaceCharacter::EndJump);
		}
		
		if (GrabAction)
		{
			EnhancedInput->BindAction(GrabAction, ETriggerEvent::Triggered, this, &APlantyRaceCharacter::Grab);
		}
		
		if (DiveAction)
		{
			EnhancedInput->BindAction(DiveAction, ETriggerEvent::Started, this, &APlantyRaceCharacter::Dive);
		}
		
		if (IA_RandomizeClothes)
		{
			EnhancedInput->BindAction(IA_RandomizeClothes, ETriggerEvent::Started, this, &APlantyRaceCharacter::RandomizeClothes);
		}
	}
}

