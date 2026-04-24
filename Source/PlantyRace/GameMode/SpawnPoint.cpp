#include "GameMode/SpawnPoint.h"

ASpawnPoint::ASpawnPoint(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = false;
}