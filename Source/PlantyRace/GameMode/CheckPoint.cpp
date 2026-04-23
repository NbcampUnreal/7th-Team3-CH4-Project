#include "GameMode/CheckPoint.h"
#include "Components/BoxComponent.h"
#include "Components/ArrowComponent.h"
#include "Components/SceneComponent.h"
#include "Core/PRGameStateBase.h"
#include "Audio/PRSoundManager.h"
#include "Actors/Characters/PlantyRacePlayerController.h"
#include "Actors/Characters/PlantyRaceCharacter.h"

ACheckPoint::ACheckPoint()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	BoxCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollision"));
	BoxCollision->SetupAttachment(SceneRoot);
	BoxCollision->SetBoxExtent(FVector(100.f, 100.f, 100.f));
	BoxCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	BoxCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	BoxCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	ArrowComponent = CreateDefaultSubobject<UArrowComponent>(TEXT("ArrowComponent"));
	ArrowComponent->SetupAttachment(SceneRoot);
}

void ACheckPoint::BeginPlay()
{
	Super::BeginPlay();

	if (BoxCollision)
	{
		BoxCollision->OnComponentBeginOverlap.AddDynamic(this, &ACheckPoint::OnCheckpointBeginOverlap);
	}
}

void ACheckPoint::OnCheckpointBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult
)
{
	if (!OtherActor)
	{
		return;
	}

	APlantyRaceCharacter* PlayerCharacter = Cast<APlantyRaceCharacter>(OtherActor);
	if (!PlayerCharacter)
	{
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Checkpoint Actor: %s / Player: %s / LastIndex: %d / ThisIndex: %d"),
		*GetName(),
		*OtherActor->GetName(),
		PlayerCharacter->GetLastCheckpointIndex(),
		CheckpointIndex);

	if (PlayerCharacter->GetLastCheckpointIndex() >= CheckpointIndex)
	{
		return;
	}

	PlayerCharacter->SetLastCheckpoint(this, CheckpointIndex);
	PlayerCharacter->SetActionState(EPlayerActionState::Idle);

	if (APlantyRacePlayerController* PRPC = Cast<APlantyRacePlayerController>(PlayerCharacter->GetController()))
	{
		PRPC->ClientPlayCheckPointSFX(GetActorLocation());
	}

	UE_LOG(LogTemp, Warning, TEXT("Checkpoint Saved: %s / CheckPointIndex: %d"),
		*GetName(),
		CheckpointIndex);
}