// Copyright © 2026 33Fellowship. All Rights Reserved.


#include "Actors/Traps/WeatherEffectZone.h"
#include "Actors/Characters/PlantyRaceCharacter.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Types/WeatherEffectTypes.h"
#include "Core/PRGameStateBase.h"

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

	TornadoParticle = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("TornadoParticle"));
	TornadoParticle->SetupAttachment(Root);
	TornadoParticle->bAutoActivate = false;
}

void AWeatherEffectZone::BeginPlay()
{
	Super::BeginPlay();

	StartLocation = GetActorLocation();
	
	GS = GetWorld() ? GetWorld()->GetGameState<APRGameStateBase>() : nullptr;
	if (!IsValid(GS))
	{
		return;
	}

	GS->OnWeatherChanged.AddUObject(this, &AWeatherEffectZone::HandleWeatherChanged);

	const bool bShouldActivate = (GS->GetCurrentWeather() == ZoneWeatherState);
	UpdateZoneStateByWeather(bShouldActivate);
}

void AWeatherEffectZone::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!HasAuthority())
	{
		return;
	}

	if (!IsValid(GS))
	{
		return;
	}

	if (ZoneWeatherState != EWeatherState::Tornado)
	{
		return;
	}

	if (GS->GetCurrentWeather() != EWeatherState::Tornado)
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
	if (!IsValid(GS))
	{
		return;
	}

	const EWeatherState CurrentWeather = GS->GetCurrentWeather();
	const bool bShouldActivate = (CurrentWeather == ZoneWeatherState);

	UpdateZoneStateByWeather(bShouldActivate);
}

void AWeatherEffectZone::UpdateZoneStateByWeather(bool bShouldActivate)
{
	UpdateGameplayState(bShouldActivate);
	UpdateVFXState(bShouldActivate);
}

void AWeatherEffectZone::UpdateGameplayState(bool bShouldActivate)
{
	if (!IsValid(BoxComp))
	{
		return;
	}

	BoxComp->SetCollisionEnabled(
		bShouldActivate ? ECollisionEnabled::QueryOnly :
		ECollisionEnabled::NoCollision
	);
}

void AWeatherEffectZone::UpdateVFXState(bool bShouldActivate)
{
	if (IsValid(ZoneMesh))
	{
		ZoneMesh->SetVisibility(bShouldActivate);
	}

	if (IsValid(TornadoParticle))
	{
		const bool bTornadoVFX = bShouldActivate && ZoneWeatherState == EWeatherState::Tornado;
		TornadoParticle->SetVisibility(bTornadoVFX);
		if (bTornadoVFX)
		{
			TornadoParticle->Activate();
		}
		else
		{
			TornadoParticle->Deactivate();
		}
	}
}
