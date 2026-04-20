#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Types/WeatherEffectTypes.h"
#include "PRWeatherManager.generated.h"

class USceneComponent;
class UNiagaraComponent;
class APRGameStateBase;

UCLASS()
class PLANTYRACE_API APRWeatherManager : public AActor
{
	GENERATED_BODY()
	
public:	
	APRWeatherManager();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Comp")
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Comp")
	TObjectPtr<UNiagaraComponent> RainComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Comp")
	TObjectPtr<UNiagaraComponent> SunComp;

	virtual void BeginPlay() override;

	UFUNCTION()
	void HandleWeatherChanged();

	void UpdateWeatherVFX(EWeatherState InCurrentWeather);

	TObjectPtr<APRGameStateBase> GS;

	EWeatherState CurrentWeather = EWeatherState::None;
};
