#include "PRWeatherManager.h"
#include "Components/SceneComponent.h"
#include "NiagaraComponent.h"
#include "Core/PRGameStateBase.h"


APRWeatherManager::APRWeatherManager()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	RainComp = CreateDefaultSubobject<UNiagaraComponent>(TEXT("RainComp"));
	RainComp->SetupAttachment(Root);
	RainComp->bAutoActivate = false;

	SunComp = CreateDefaultSubobject<UNiagaraComponent>(TEXT("SunComp"));
	SunComp->SetupAttachment(Root);
	SunComp->bAutoActivate = false;
}

void APRWeatherManager::BeginPlay()
{
	Super::BeginPlay();
	
	GS = GetWorld() ? GetWorld()->GetGameState<APRGameStateBase>() : nullptr;
	if (!IsValid(GS))
	{
		return;
	}

	GS->OnWeatherChanged.AddUObject(this, &APRWeatherManager::HandleWeatherChanged);

	CurrentWeather = GS->GetCurrentWeather();

	UpdateWeatherVFX(CurrentWeather);
}

void APRWeatherManager::HandleWeatherChanged()
{
	if (!IsValid(GS))
	{
		return;
	}

	CurrentWeather = GS->GetCurrentWeather();
	UpdateWeatherVFX(CurrentWeather);
}

void APRWeatherManager::UpdateWeatherVFX(EWeatherState InCurrentWeather)
{
	bool bShouldRain = false;
	bool bShouldSun = false;

	if (InCurrentWeather == EWeatherState::Rain)
	{
		bShouldRain = true;
	}

	if (InCurrentWeather == EWeatherState::Sun)
	{
		bShouldSun = true;
	}

	if (IsValid(RainComp))
	{
		RainComp->SetVisibility(bShouldRain);
		if (bShouldRain)
		{
			RainComp->Activate();
		}
		else
		{
			RainComp->Deactivate();
		}
	}

	if (IsValid(SunComp))
	{
		SunComp->SetVisibility(bShouldSun);
		if (bShouldSun)
		{
			SunComp->Activate();
		}
		else
		{
			SunComp->Deactivate();
		}
	}
}
