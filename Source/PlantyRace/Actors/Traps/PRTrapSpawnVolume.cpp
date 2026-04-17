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

	if (RollingActorClass.Num() == 0) // 배열이 비어있으면 타이머 실행중지 (오류 방지)
	{
		return;
	}

	if (SpawnInterval <= 0.f)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (World)
	{
		World->GetTimerManager().SetTimer( 
			SpawnTimerHandle,
			this,
			&APRTrapSpawnVolume::SpawnRollingActor,
			SpawnInterval,
			true
		);
	}
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
	if (!HasAuthority() || RollingActorClass.Num() == 0) return;

	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	// 배열에서 랜덤하게 하나 선택
	int32 RandomIndex = FMath::RandRange(0, RollingActorClass.Num() - 1);
	TSubclassOf<APRRollingActor> SelectedClass = RollingActorClass[RandomIndex];

	if (SelectedClass == nullptr) 
	{
		return;
	}


	FVector SpawnLocation = GetRandomPointVolume();
	FRotator SpawnRotation = FRotator::ZeroRotator;

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	
	World->SpawnActor<APRRollingActor>(
		SelectedClass,
		SpawnLocation,
		SpawnRotation,
		SpawnParams
	);
}

