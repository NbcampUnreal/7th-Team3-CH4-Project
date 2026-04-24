#include "PRTornadoComponent.h"
#include "Actors/Characters/PlantyRaceCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Actors/Traps/WeatherEffectZone.h"
#include "Net/UnrealNetwork.h"

UPRTornadoComponent::UPRTornadoComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
    SetIsReplicatedByDefault(true);
    
}

void UPRTornadoComponent::BeginPlay()
{
	Super::BeginPlay();
	
    OwnerCharacter = Cast<APlantyRaceCharacter>(GetOwner());
    
}

void UPRTornadoComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ThisClass, bInTornado);
    DOREPLIFETIME(ThisClass, TornadoSourceActor);
    DOREPLIFETIME(ThisClass, CurrentTornadoZone);
}

void UPRTornadoComponent::EnterTornado(AActor* InTornadoSource)
{
    if (!OwnerCharacter || !OwnerCharacter->HasAuthority())
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

    UCharacterMovementComponent* MoveComp = OwnerCharacter->GetCharacterMovement();
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

void UPRTornadoComponent::ExitTornado()
{
    if (!OwnerCharacter || !OwnerCharacter->HasAuthority())
    {
        return;
    }

    if (!bInTornado)
    {
        return;
    }

    UCharacterMovementComponent* MoveComp = OwnerCharacter->GetCharacterMovement();
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

bool UPRTornadoComponent::IsRisePhase() const
{
    return TornadoElapsedTime < TornadoRiseDuration;
}

bool UPRTornadoComponent::IsTornadoFinished() const
{
    return TornadoElapsedTime >= TornadoTotalDuration;
}

FVector UPRTornadoComponent::GetSuctionVelocity(const FVector& ToCenter) const
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

FVector UPRTornadoComponent::GetOrbitVelocity(const FVector& ToCenter) const
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

FVector UPRTornadoComponent::GetVerticalVelocity() const
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

void UPRTornadoComponent::UpdateTornadoMovement(float DeltaTime)
{
    if (!bInTornado || !IsValid(TornadoSourceActor))
    {
        return;
    }

    UCharacterMovementComponent* MoveComp = OwnerCharacter->GetCharacterMovement();
    if (!IsValid(MoveComp))
    {
        return;
    }

    TornadoElapsedTime += DeltaTime;

    FVector ToCenter = TornadoSourceActor->GetActorLocation() - OwnerCharacter->GetActorLocation();
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

void UPRTornadoComponent::OnRep_InTornado()
{
    if (!IsValid(OwnerCharacter))
    {
        return;
    }

    UCharacterMovementComponent* MoveComp = OwnerCharacter->GetCharacterMovement();
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