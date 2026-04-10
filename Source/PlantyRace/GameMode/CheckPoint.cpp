#include "GameMode/CheckPoint.h"
#include "Components/BoxComponent.h"
#include "Components/ArrowComponent.h"
#include "Components/SceneComponent.h"

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

	UE_LOG(LogTemp, Warning, TEXT("Checkpoint Overlap: %s entered CheckPoint %d"),
		*OtherActor->GetName(),
		CheckpointIndex);
}