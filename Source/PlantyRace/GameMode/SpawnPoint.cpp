#include "GameMode/SpawnPoint.h"
#include "Components/ArrowComponent.h"
#include "Components/SceneComponent.h"

ASpawnPoint::ASpawnPoint()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	ArrowComponent = CreateDefaultSubobject<UArrowComponent>(TEXT("ArrowComponent"));
	ArrowComponent->SetupAttachment(SceneRoot);
}