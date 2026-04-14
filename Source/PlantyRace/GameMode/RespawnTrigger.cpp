#include "GameMode/RespawnTrigger.h"
#include "Components/BoxComponent.h"
#include "Actors/Characters/PlantyRaceCharacter.h"

ARespawnTrigger::ARespawnTrigger()
{
	PrimaryActorTick.bCanEverTick = false;

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	SetRootComponent(TriggerBox);

	TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}

void ARespawnTrigger::BeginPlay()
{
	Super::BeginPlay();

	TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &ARespawnTrigger::OnOverlapBegin);
}

void ARespawnTrigger::OnOverlapBegin(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult
)
{
	APlantyRaceCharacter* Player = Cast<APlantyRaceCharacter>(OtherActor);

	if (!Player)
		return;

	UE_LOG(LogTemp, Warning, TEXT("Player Fell! Respawn Triggered"));
}