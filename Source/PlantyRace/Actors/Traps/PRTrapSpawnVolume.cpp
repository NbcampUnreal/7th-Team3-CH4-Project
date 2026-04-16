#include "Actors/Traps/PRTrapSpawnVolume.h"
#include "Components/BoxComponent.h"
#include "PRRollingActor.h"

APRTrapSpawnVolume::APRTrapSpawnVolume()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	BoxComp = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComp"));
	SetRootComponent(BoxComp);
	BoxComp->SetBoxExtent(BoxExtent);
	BoxComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void APRTrapSpawnVolume::BeginPlay()
{
	Super::BeginPlay();
	
	if (!HasAuthority())
	{
		return;
	}

	if (RollingActorClass == nullptr)
	{
		return;
	}

	if (SpawnInterval <= 0.f)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	World->GetTimerManager().SetTimer(
		SpawnTimerHandle,
		this,
		&APRTrapSpawnVolume::SpawnRollingActor,
		SpawnInterval,
		true
	);
}

FVector APRTrapSpawnVolume::GetRandomPointVolume() const
{
	if (!BoxComp)
	{
		return FVector::ZeroVector;
	}

	FVector Origin = BoxComp->GetComponentLocation();
	FVector Extent = BoxComp->GetScaledBoxExtent();

	float RandomX = FMath::RandRange(-Extent.X, Extent.X);
	float RandomY = FMath::RandRange(-Extent.Y, Extent.Y);
	float RandomZ = FMath::RandRange(-Extent.Z, Extent.Z);

	FVector RandomOffset = FVector(RandomX, RandomY, RandomZ);

	return Origin + RandomOffset;
}

void APRTrapSpawnVolume::SpawnRollingActor()
{
	if (!HasAuthority())
	{
		return;
	}

	if (RollingActorClass == nullptr) 
	{
		return;
	}

	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	FVector SpawnLocation = GetRandomPointVolume();
	FRotator SpawnRotation = FRotator::ZeroRotator;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	
	World->SpawnActor<APRRollingActor>(
		RollingActorClass,
		SpawnLocation,
		SpawnRotation,
		SpawnParams
	);
}

