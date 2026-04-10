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
#include "Kismet/GameplayStatics.h"
#include "Core/PRGameStateBase.h"
#include "Components/CharacterEffectComponent.h"
#include "Actors/Traps/WeatherEffectZone.h"

APlantyRaceCharacter::APlantyRaceCharacter()
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

    MouseSensitivity = 1.5f;
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

void APlantyRaceCharacter::RandomizeClothes()
{
    SetRandomMesh(PantsSkeletalMesh, PantsOptions);
    SetRandomMesh(ShirtSkeletalMesh, ShirtOptions);
    SetRandomMesh(HairSkeletalMesh, HairOptions);
    SetRandomMesh(ShoeSkeletalMesh, ShoeOptions);
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
    DOREPLIFETIME(ThisClass, bInTornado);
    DOREPLIFETIME(ThisClass, TornadoSourceActor);
    DOREPLIFETIME(ThisClass, CurrentTornadoZone);
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
            EnhancedInput->BindAction(GrabAction, ETriggerEvent::Triggered, this, &APlantyRaceCharacter::Grab);
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

bool APlantyRaceCharacter::IsRisePhase() const
{
    return TornadoElapsedTime < TornadoRiseDuration;
}

bool APlantyRaceCharacter::IsTornadoFinished() const
{
    return TornadoElapsedTime >= TornadoTotalDuration;
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
