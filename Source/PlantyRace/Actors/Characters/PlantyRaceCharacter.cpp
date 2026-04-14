#include "PlantyRaceCharacter.h"
#include <ThirdParty/ShaderConductor/ShaderConductor/External/DirectXShaderCompiler/include/dxc/DXIL/DxilConstants.h>
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "Core/PRGameStateBase.h"
#include "Components/CharacterEffectComponent.h"
#include "Actors/Traps/WeatherEffectZone.h"
#include "Curves/CurveFloat.h"
#include "PRCharacterMovementComponent.h"
#include "DrawDebugHelpers.h"
#include "Components/CapsuleComponent.h"
#include "GameMode/CheckPoint.h"
#include "GameMode/SpawnPoint.h"
#include "Components/PRKnockbackComponent.h"

APlantyRaceCharacter::APlantyRaceCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(
	ObjectInitializer.SetDefaultSubobjectClass<
	UPRCharacterMovementComponent>(
	ACharacter::CharacterMovementComponentName))
{
    PrimaryActorTick.bCanEverTick = true;

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

	GrabHoldPoint = CreateDefaultSubobject<USceneComponent>(TEXT("GrabHoldPoint"));
	GrabHoldPoint->SetupAttachment(RootComponent);
	GrabHoldPoint->SetRelativeLocation(FVector(100.f, 0.f, 50.f));
	
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

    CharacterEffectComp = CreateDefaultSubobject<UCharacterEffectComponent>(TEXT("CharacterEffectComp"));

    KnockbackComp = CreateDefaultSubobject<UPRKnockbackComponent>(TEXT("KnockbackComp"));

    MouseSensitivity = 1.5f;
	
	GrabTarget = nullptr;
	GrabbedBy = nullptr;
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
    if (bInTornado)
    {
        return;
    }
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

void APlantyRaceCharacter::StartGrab(const FInputActionValue& Value)
{
	if (!CanGrabAction())
	{
		return;
	}

	ServerGrab();

	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		if (GrabMontage)
		{
			AnimInstance->Montage_Play(GrabMontage);
		}
	}
}

void APlantyRaceCharacter::EndGrab(const FInputActionValue& Value)
{
	ServerRelease();
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

    if (IsKnockedDown())
    {
        return;
    }

	SetActionState(EPlayerActionState::Idle);
}

void APlantyRaceCharacter::ServerGrab_Implementation()
{
	if (!CanGrabAction())
	{
		return;
	}
	
	if (GrabTarget)
	{
		return;
	}

    FVector Start = GetActorLocation();
    FVector End = Start + GetActorForwardVector() * 30.f;

    FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	const bool bHit =  GetWorld()->SweepSingleByChannel(
        Hit,
        Start,
        End,
        FQuat::Identity,
        ECC_Pawn,
        FCollisionShape::MakeSphere(50.f),
        Params
    );
	FColor Color = bHit ? FColor::Red : FColor::Green;

	DrawDebugSphere(GetWorld(), End, 50.f, 16, Color, false, 2.f);
	DrawDebugLine(GetWorld(), Start, End, Color, false, 2.f);
	if (!bHit)
	{
		return;
	}
    APlantyRaceCharacter* Target = Cast<APlantyRaceCharacter>(Hit.GetActor());
    if (!Target)
    {
       return;
    }
	if (Target == this)
	{
		return;
	}
	if (Target->bIsGrabbed)
	{
		return;
	}
	GrabTarget = Target;
	Target->bIsGrabbed = true;
	Target->GrabbedBy = this;

	if (UCharacterMovementComponent* MoveComp = Target->GetCharacterMovement())
	{
		MoveComp->DisableMovement();
	}

	Target->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	Target->AttachToComponent(
	GrabHoldPoint,
		FAttachmentTransformRules::SnapToTargetNotIncludingScale
	);
}

void APlantyRaceCharacter::ServerRelease_Implementation()
{
	if (!GrabTarget)
	{
		return;
	}
	GrabTarget->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	
	if(UCharacterMovementComponent* MoveComp = GrabTarget->GetCharacterMovement())	
	{
		MoveComp->SetMovementMode(MOVE_Walking);
	}
	GrabTarget->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	
	GrabTarget->bIsGrabbed = false;
	GrabTarget->GrabbedBy = nullptr;
	GrabTarget = nullptr;
}

// Called when the game starts or when spawned
void APlantyRaceCharacter::BeginPlay()
{
    Super::BeginPlay();

    InitializeModularMeshes();
    ApplyClothesFromRepData();

    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (!IsValid(MoveComp))
    {
        return;
    }

    DefaultMaxWalkSpeed = MoveComp->MaxWalkSpeed;
    DefaultJumpZVelocity = MoveComp->JumpZVelocity;

    if (!IsValid(CharacterEffectComp))
    {
        return;
    }

    CharacterEffectComp->InitializeEffects();

    APRGameStateBase* PGS = GetWorld() ? GetWorld()->GetGameState<APRGameStateBase>() : nullptr;
    if (!IsValid(PGS))
    {
        return;
    }

    PGS->OnWeatherChanged.AddUObject(this, &APlantyRaceCharacter::HandleWeatherChanged);
    HandleWeatherChanged();
}

// Called every frame
void APlantyRaceCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bInTornado && HasAuthority())
    {
        UpdateTornadoMovement(DeltaTime);
    }
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

    DOREPLIFETIME(ThisClass, bIsGrabbed);
    DOREPLIFETIME(ThisClass, bInTornado);
    DOREPLIFETIME(ThisClass, TornadoSourceActor);
    DOREPLIFETIME(ThisClass, CurrentTornadoZone);
	DOREPLIFETIME(ThisClass, ClothesData);
	DOREPLIFETIME(APlantyRaceCharacter, GrabTarget);
	DOREPLIFETIME(APlantyRaceCharacter, GrabbedBy);
    DOREPLIFETIME(ThisClass, bIsKnockedDown);
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

    float EffectMultiplier = 1.f;
    if (IsValid(CharacterEffectComp))
    {
        EffectMultiplier = CharacterEffectComp->GetMoveSpeedMultiplier();
    }

    MoveComp->MaxWalkSpeed = FinalSpeed * EffectMultiplier;
}


bool APlantyRaceCharacter::CanMove() const
{
	return CurrentActionState != EPlayerActionState::Dive
		&& CurrentActionState != EPlayerActionState::Attack
		&& CurrentActionState != EPlayerActionState::Slide
        && !IsKnockedDown();
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

    if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent))
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
    		EnhancedInput->BindAction(GrabAction, ETriggerEvent::Started, this, &APlantyRaceCharacter::StartGrab);
    		EnhancedInput->BindAction(GrabAction, ETriggerEvent::Completed, this, &APlantyRaceCharacter::EndGrab);
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

bool APlantyRaceCharacter::CanJumpInternal_Implementation() const
{
    if (!IsValid(CharacterEffectComp))
    {
        return Super::CanJumpInternal_Implementation();
    }

    if (bInTornado)
    {
        return false;
    }

    if (CharacterEffectComp->GetBlockJump())
    {
        return false;
    }

    if (IsKnockedDown())
    {
        return false;
    }

    return Super::CanJumpInternal_Implementation();
}

void APlantyRaceCharacter::EnterWeatherZone(ESurfaceEffectType InSurfaceEffectType, float InMoveSpeedMultiplier, float InJumpMultiplier, bool bInBlockJump)
{
    if (!IsValid(CharacterEffectComp))
    {
        return;
    }

    CharacterEffectComp->ApplyZoneEffect(InSurfaceEffectType, InMoveSpeedMultiplier, InJumpMultiplier, bInBlockJump);
}

void APlantyRaceCharacter::ExitWeatherZone()
{
    if (!IsValid(CharacterEffectComp))
    {
        return;
    }

    CharacterEffectComp->ClearZoneEffect();
}

void APlantyRaceCharacter::EnterTornado(AActor* InTornadoSource)
{
    if (!HasAuthority())
    {
        return;
    }

    if (bInTornado)
    {
        return;
    }

    if (!IsValid(InTornadoSource))
    {
        return;
    }

    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (!IsValid(MoveComp))
    {
        return;
    }

    AWeatherEffectZone* NewZone = Cast<AWeatherEffectZone>(InTornadoSource);
    if (!IsValid(NewZone))
    {
        return;
    }

    bInTornado = true;
    TornadoElapsedTime = 0.0f;
    TornadoSourceActor = InTornadoSource;
    MoveComp->Velocity = FVector::ZeroVector;
    MoveComp->SetMovementMode(MOVE_Falling);
    CurrentTornadoZone = NewZone;
}

void APlantyRaceCharacter::ExitTornado()
{
    if (!HasAuthority())
    {
        return;
    }

    if (!bInTornado)
    {
        return;
    }

    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (!IsValid(MoveComp))
    {
        return;
    }

    bInTornado = false;
    TornadoElapsedTime = 0.0f;
    TornadoSourceActor = nullptr;
    MoveComp->Velocity = FVector::ZeroVector;
    CurrentTornadoZone = nullptr;

    if (MoveComp->IsMovingOnGround())
    {
        MoveComp->SetMovementMode(MOVE_Walking);
    }
    else
    {
        MoveComp->SetMovementMode(MOVE_Falling);
    }
}

void APlantyRaceCharacter::PlayFootstepSound(EFootType FootType)
{
    if (!IsValid(CharacterEffectComp))
    {
        return;
    }

    if (!IsValid(PuddleFootstepSound))
    {
        return;
    }

    if (CharacterEffectComp->GetCurrentSurfaceEffect() != ESurfaceEffectType::Puddle)
    {
        return;
    }

    USkeletalMeshComponent* MeshComp = GetMesh();
    if (!IsValid(MeshComp))
    {
        return;
    }

    FName TargetSocketName;

    if (FootType == EFootType::Left)
    {
        TargetSocketName = LeftFootstepSocketName;
    }
    else
    {
        TargetSocketName = RightFootstepSocketName;
    }

    if (TargetSocketName.IsNone())
    {
        return;
    }

    FVector SoundLocation = GetActorLocation();

    if (MeshComp->DoesSocketExist(TargetSocketName))
    {
        SoundLocation = MeshComp->GetSocketLocation(TargetSocketName);
    }

    UGameplayStatics::PlaySoundAtLocation(
        this,
        PuddleFootstepSound,
        SoundLocation
    );
}

void APlantyRaceCharacter::SetKnockedDown(bool bValue)
{
    if (bIsKnockedDown == bValue)
    {
        return;
    }

    bIsKnockedDown = bValue;

    HandleKnockedDownChanged();
}

bool APlantyRaceCharacter::IsRisePhase() const
{
    return TornadoElapsedTime < TornadoRiseDuration;
}

bool APlantyRaceCharacter::IsTornadoFinished() const
{
    return TornadoElapsedTime >= TornadoTotalDuration;
}

bool APlantyRaceCharacter::IsKnockedDown() const
{
    return bIsKnockedDown;
}

FVector APlantyRaceCharacter::GetSuctionVelocity(const FVector& ToCenter) const
{
    FVector ToCenterXY = FVector(ToCenter.X, ToCenter.Y, 0.f);

    if (ToCenterXY.IsNearlyZero())
    {
        return FVector::ZeroVector;
    }

    if (FMath::IsNearlyEqual(TornadoMinSuctionDistance, TornadoMaxSuctionDistance))
    {
        return FVector::ZeroVector;
    }

    const FVector SuctionDirection = ToCenterXY.GetSafeNormal();
    const float DistanceToCenter = ToCenterXY.Size();

    const float ClampedDistance = FMath::Clamp(DistanceToCenter, TornadoMinSuctionDistance, TornadoMaxSuctionDistance);
    const float Alpha = (ClampedDistance - TornadoMinSuctionDistance) / (TornadoMaxSuctionDistance - TornadoMinSuctionDistance);
    const float EasedAlpha = FMath::InterpEaseInOut(0.f, 1.f, Alpha, TornadoSuctionEaseExponent);
    const float SmoothedScale = FMath::Lerp(TornadoMinSuctionScale, TornadoMaxSuctionScale, EasedAlpha);
    const float FinalSuctionSpeed = TornadoSuctionSpeed * SmoothedScale;

    return SuctionDirection * FinalSuctionSpeed;
}

FVector APlantyRaceCharacter::GetOrbitVelocity(const FVector& ToCenter) const
{
    FVector ToCenterXY = FVector(ToCenter.X, ToCenter.Y, 0.f);

    if (ToCenterXY.IsNearlyZero())
    {
        return FVector::ZeroVector;
    }

    FVector CenterDirection = ToCenterXY.GetSafeNormal();
    FVector OrbitDirection = FVector(-CenterDirection.Y, CenterDirection.X, 0.f);

    return OrbitDirection * TornadoOrbitSpeed;
}

FVector APlantyRaceCharacter::GetVerticalVelocity() const
{
    FVector VerticalVelocity = FVector::ZeroVector;

    if (IsRisePhase())
    {
        VerticalVelocity.Z = TornadoRiseSpeed;
    }
    else if (!IsTornadoFinished())
    {
        VerticalVelocity.Z = TornadoFallSpeed;
    }
    else
    {
        VerticalVelocity = FVector::ZeroVector;
    }

    return VerticalVelocity;
}

void APlantyRaceCharacter::PlayKnockedDownMontage()
{
    USkeletalMeshComponent* MeshComp = GetMesh();
    if (!IsValid(MeshComp))
    {
        return;
    }

    UAnimInstance* AI = MeshComp->GetAnimInstance();
    if (!IsValid(AI))
    {
        return;
    }

    if (!KnockedDownMontage)
    {
        return;
    }

    AI->Montage_Play(KnockedDownMontage);
}

void APlantyRaceCharacter::PlayGetUpMontage()
{
    USkeletalMeshComponent* MeshComp = GetMesh();
    if (!IsValid(MeshComp))
    {
        return;
    }

    UAnimInstance* AI = MeshComp->GetAnimInstance();
    if (!IsValid(AI))
    {
        return;
    }

    if (!GetUpMontage)
    {
        return;
    }

    AI->Montage_Play(GetUpMontage);
}

void APlantyRaceCharacter::LockMovement()
{
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (!IsValid(MoveComp))
    {
        return;
    }

    MoveComp->DisableMovement();
}

void APlantyRaceCharacter::UnlockMovement()
{
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (!IsValid(MoveComp))
    {
        return;
    }

    if (MoveComp->IsMovingOnGround())
    {
        MoveComp->SetMovementMode(MOVE_Walking);
    }
    else
    {
        MoveComp->SetMovementMode(MOVE_Falling);
    }
}

void APlantyRaceCharacter::OnRep_InTornado()
{
    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (!IsValid(MoveComp))
    {
        return;
    }

    if (bInTornado)
    {
        MoveComp->Velocity = FVector::ZeroVector;
        MoveComp->SetMovementMode(MOVE_Falling);
    }
    else
    {
        TornadoElapsedTime = 0.0f;

        if (MoveComp->IsMovingOnGround())
        {
            MoveComp->SetMovementMode(MOVE_Walking);
        }
        else
        {
            MoveComp->SetMovementMode(MOVE_Falling);
        }
    }
}

void APlantyRaceCharacter::OnRep_IsKnockedDown()
{
    HandleKnockedDownChanged();
}

void APlantyRaceCharacter::HandleWeatherChanged()
{
    APRGameStateBase* PGS = GetWorld() ? GetWorld()->GetGameState<APRGameStateBase>() : nullptr;
    if (!IsValid(PGS))
    {
        return;
    }

    if (!IsValid(CharacterEffectComp))
    {
        return;
    }

    const EWeatherState NewWeather = PGS->GetCurrentWeather();

    CharacterEffectComp->SetWeatherState(NewWeather);
}

void APlantyRaceCharacter::HandleKnockedDownChanged()
{
    if (bIsKnockedDown)
    {
        StopJumping();
        SetActionState(EPlayerActionState::KnockedDown);
        PlayKnockedDownMontage();
    }
    else
    {
        SetActionState(EPlayerActionState::Idle); // AnimMontage 추가시 제거
        PlayGetUpMontage();
    }
}

void APlantyRaceCharacter::UpdateTornadoMovement(float DeltaTime)
{
    if (!bInTornado || !IsValid(TornadoSourceActor))
    {
        return;
    }

    UCharacterMovementComponent* MoveComp = GetCharacterMovement();
    if (!IsValid(MoveComp))
    {
        return;
    }

    TornadoElapsedTime += DeltaTime;

    FVector ToCenter = TornadoSourceActor->GetActorLocation() - GetActorLocation();
    FVector NewVelocity = FVector::ZeroVector;

    FVector VerticalVelocity = GetVerticalVelocity();
    FVector SuctionVelocity = GetSuctionVelocity(ToCenter);
    FVector OrbitVelocity = GetOrbitVelocity(ToCenter);

    NewVelocity += VerticalVelocity;
    NewVelocity += SuctionVelocity;
    NewVelocity += OrbitVelocity;

    MoveComp->Velocity = NewVelocity;

    if (IsTornadoFinished())
    {
        ExitTornado();
    }
}

void APlantyRaceCharacter::SetLastCheckpoint(ACheckPoint* NewCheckpoint, int32 NewCheckpointIndex)
{
    if (!NewCheckpoint)
    {
        return;
    }

    if (NewCheckpointIndex < LastCheckpointIndex)
    {
        return;
    }

    LastCheckpoint = NewCheckpoint;
    LastCheckpointIndex = NewCheckpointIndex;
}

void APlantyRaceCharacter::SetStartSpawnPoint(ASpawnPoint* NewSpawnPoint)
{
    if (!NewSpawnPoint)
    {
        return;
    }

    StartSpawnPoint = NewSpawnPoint;
}