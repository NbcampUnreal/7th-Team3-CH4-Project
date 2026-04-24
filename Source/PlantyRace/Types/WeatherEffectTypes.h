#pragma once

#include "CoreMinimal.h"
#include "WeatherEffectTypes.generated.h"

UENUM(BlueprintType)
enum class EWeatherState : uint8
{
    None    UMETA(DisplayName = "None"),
    Sun     UMETA(DisplayName = "Sun"),
    Rain    UMETA(DisplayName = "Rain"),
    Tornado UMETA(DisplayName = "Tornado")
};

UENUM(BlueprintType)
enum class ESurfaceEffectType : uint8
{
    None    UMETA(DisplayName = "None"),
    Mud     UMETA(DisplayName = "Mud"),
    Puddle  UMETA(DisplayName = "Puddle")
};