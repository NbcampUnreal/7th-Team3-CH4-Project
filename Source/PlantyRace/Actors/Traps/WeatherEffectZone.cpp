// Copyright © 2026 33Fellowship. All Rights Reserved.


#include "Actors/Traps/WeatherEffectZone.h"
#include "Actors/Characters/PlantyRaceCharacter.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Types/WeatherEffectTypes.h"

AWeatherEffectZone::AWeatherEffectZone()
{
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;
	SetReplicateMovement(true);

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	BoxComp = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComp"));
	BoxComp->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	BoxComp->SetupAttachment(Root);
	BoxComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	BoxComp->SetGenerateOverlapEvents(true);
	BoxComp->OnComponentBeginOverlap.AddDynamic(this, &AWeatherEffectZone::OnZoneBeginOverlap);
	BoxComp->OnComponentEndOverlap.AddDynamic(this, &AWeatherEffectZone::OnZoneEndOverlap);

	ZoneMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ZoneMesh"));
	ZoneMesh->SetupAttachment(Root);
	ZoneMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	ZoneMesh->SetVisibility(false);

}

void AWeatherEffectZone::BeginPlay()
{
	Super::BeginPlay();

	StartLocation = GetActorLocation();
	
	APRGameStateBase* PGS = GetWorld() ? GetWorld()->GetGameState<APRGameStateBase>() : nullptr;
	if (!IsValid(PGS))
	{
		return;
	}

	PGS->OnWeatherChanged.AddUObject(this, &AWeatherEffectZone::HandleWeatherChanged);

	const bool bShouldActivate = (PGS->GetCurrentWeather() == ZoneWeatherState);
	UpdateZoneVisibilityByWeather(bShouldActivate);
}

void AWeatherEffectZone::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!HasAuthority())
	{
		return;
	}

	APRGameStateBase* PGS = GetWorld() ? GetWorld()->GetGameState<APRGameStateBase>() : nullptr;
	if (!IsValid(PGS))
	{
		return;
	}

	if (ZoneWeatherState != EWeatherState::Tornado)
	{
		return;
	}

	if (PGS->GetCurrentWeather() != EWeatherState::Tornado)
	{
		return;
	}

	if (!bMoveTornado)
	{
		return;
	}

	if (MoveDirection.IsNearlyZero())
	{
		return;
	}

	FVector NormalDirection = MoveDirection.GetSafeNormal();

	float FrameMoveDistance = MoveSpeed * DeltaTime;
	FVector MoveOffset = NormalDirection * FrameMoveDistance;
	FVector NextLocation = GetActorLocation() + MoveOffset;

	float DistanceFromStart = FVector::Dist(StartLocation, NextLocation);
	bool bOutOfRange = DistanceFromStart >= MoveRange;

	if (bOutOfRange)
	{
		MoveDirection = -MoveDirection;
	}
	else
	{
		SetActorLocation(NextLocation);
	}
}

void AWeatherEffectZone::OnZoneBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority())
	{
		return;
	}

	APlantyRaceCharacter* Character = Cast<APlantyRaceCharacter>(OtherActor);
	if (!IsValid(Character))
	{
		return;
	}

	if (ZoneWeatherState == EWeatherState::Tornado)
	{
		Character->EnterTornado(this);
		return;
	}

	Character->EnterWeatherZone(
		SurfaceEffectType,
		MoveSpeedMultiplier,
		JumpMultiplier,
		bBlockJump
	);
}

void AWeatherEffectZone::OnZoneEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!HasAuthority())
	{
		return;
	}

	APlantyRaceCharacter* Character = Cast<APlantyRaceCharacter>(OtherActor);
	if (!IsValid(Character))
	{
		return;
	}

	if (ZoneWeatherState == EWeatherState::Tornado)
	{
		if (Character->GetCurrentTornadoZone() == this)
		{
			Character->ExitTornado();
		}
		return;
	}
	
	Character->ExitWeatherZone();
}

void AWeatherEffectZone::HandleWeatherChanged()
{
	APRGameStateBase* PGS = GetWorld() ? GetWorld()->GetGameState<APRGameStateBase>() : nullptr;
	if (!IsValid(PGS))
	{
		return;
	}

	const EWeatherState CurrentWeather = PGS->GetCurrentWeather();
	const bool bShouldActivate = (CurrentWeather == ZoneWeatherState);

	UpdateZoneVisibilityByWeather(bShouldActivate);
}

void AWeatherEffectZone::UpdateZoneVisibilityByWeather(bool bShouldActivate)
{
	if (!IsValid(BoxComp) || !IsValid(ZoneMesh))
	{
		return;
	}

	ZoneMesh->SetVisibility(bShouldActivate);
	BoxComp->SetCollisionEnabled(
		bShouldActivate ? ECollisionEnabled::QueryOnly :
		ECollisionEnabled::NoCollision
	);
}
