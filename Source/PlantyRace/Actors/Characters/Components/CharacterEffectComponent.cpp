#include "CharacterEffectComponent.h"
#include "Actors/Characters/PlantyRaceCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

UCharacterEffectComponent::UCharacterEffectComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCharacterEffectComponent::BeginPlay()
{
	Super::BeginPlay();

	APlantyRaceCharacter* Character = Cast<APlantyRaceCharacter>(GetOwner());
	if (!IsValid(Character))
	{
		return;
	}

	UCharacterMovementComponent* MoveComp = Character->GetCharacterMovement();
	if (!IsValid(MoveComp))
	{
		return;
	}

	DefaultGroundFriction = MoveComp->GroundFriction;
	DefaultBrakingDecelerationWalking = MoveComp->BrakingDecelerationWalking;
}

float UCharacterEffectComponent::GetMoveSpeedMultiplier() const
{
	return CachedMoveSpeedMultiplier;
}

float UCharacterEffectComponent::GetJumpMultiplier() const
{
	return CachedJumpMultiplier;
}

bool UCharacterEffectComponent::GetBlockJump() const
{
	return bCachedBlockJump;
}

ESurfaceEffectType UCharacterEffectComponent::GetCurrentSurfaceEffect() const
{
	return CachedSurfaceEffectType;
}

void UCharacterEffectComponent::SetWeatherState(EWeatherState NewWeatherState)
{
	CurrentWeather = NewWeatherState;
	UpdateWeatherModifiers();
	RecalculateEffects();
}

void UCharacterEffectComponent::ApplyZoneEffect(ESurfaceEffectType InSurfaceEffectType, float InMoveSpeedMultiplier, float InJumpMultiplier, bool bInBlockJump)
{
	CurrentSurfaceEffect = InSurfaceEffectType;
	ZoneMoveSpeedMultiplier = InMoveSpeedMultiplier;
	ZoneJumpMultiplier = InJumpMultiplier;
	bZoneBlockJump = bInBlockJump;

	RecalculateEffects();
}

void UCharacterEffectComponent::ClearZoneEffect()
{
	CurrentSurfaceEffect = ESurfaceEffectType::None;
	bZoneBlockJump = false;
	ZoneMoveSpeedMultiplier = 1.f;
	ZoneJumpMultiplier = 1.f;

	RecalculateEffects();
}

void UCharacterEffectComponent::InitializeEffects()
{
	RecalculateEffects();
}

void UCharacterEffectComponent::UpdateWeatherModifiers()
{
	WeatherMoveSpeedMultiplier = 1.0f;
	WeatherJumpMultiplier = 1.0f;
	bWeatherBlockJump = false;

	switch (CurrentWeather)
	{
	case EWeatherState::None:
		WeatherMoveSpeedMultiplier = 1.f;
		WeatherJumpMultiplier = 1.f;
		break;
	case EWeatherState::Rain:
		WeatherMoveSpeedMultiplier = 1.f;
		WeatherJumpMultiplier = 1.f;
		break;
	case EWeatherState::Sun:
		WeatherMoveSpeedMultiplier = 1.5f;
		WeatherJumpMultiplier = 1.f;
		bWeatherBlockJump = true;
		break;
	default:
		WeatherMoveSpeedMultiplier = 1.0f;
		WeatherJumpMultiplier = 1.0f;
		break;
	}
}

void UCharacterEffectComponent::RecalculateEffects()
{
	CachedSurfaceEffectType = CurrentSurfaceEffect;
	CachedMoveSpeedMultiplier = WeatherMoveSpeedMultiplier * ZoneMoveSpeedMultiplier;
	CachedJumpMultiplier = WeatherJumpMultiplier * ZoneJumpMultiplier;
	bCachedBlockJump = bWeatherBlockJump || bZoneBlockJump;

	ApplyMovementPhysics();
}

void UCharacterEffectComponent::ApplyMovementPhysics()
{
	APlantyRaceCharacter* Character = Cast<APlantyRaceCharacter>(GetOwner());
	if (!IsValid(Character))
	{
		return;
	}

	UCharacterMovementComponent* MoveComp = Character->GetCharacterMovement();
	if (!IsValid(MoveComp))
	{
		return;
	}

	MoveComp->GroundFriction = DefaultGroundFriction;
	MoveComp->BrakingDecelerationWalking = DefaultBrakingDecelerationWalking;

	MoveComp->MaxWalkSpeed = Character->GetDefaultMaxWalkSpeed() * CachedMoveSpeedMultiplier;
	MoveComp->JumpZVelocity = Character->GetDefaultJumpZVelocity() * CachedJumpMultiplier;

	if (CurrentWeather == EWeatherState::Rain)
	{
		MoveComp->GroundFriction = RainGroundFriction;
		MoveComp->BrakingDecelerationWalking = RainBrakingDecelerationWalking;
	}
}
