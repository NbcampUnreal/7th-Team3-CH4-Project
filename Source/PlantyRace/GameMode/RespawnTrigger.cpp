#include "GameMode/RespawnTrigger.h"
#include "Components/BoxComponent.h"
#include "Actors/Characters/PlantyRaceCharacter.h"
#include "GameMode/PRGMB.h"
#include "Engine/World.h"

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

void AFinishTrigger::OnFinishTriggerBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult
)
{
	if (!HasAuthority())
	{
		return;
	}

	APlantyRaceCharacter* PlayerCharacter = Cast<APlantyRaceCharacter>(OtherActor);
	if (!PlayerCharacter)
	{
		return;
	}

	APRPlayerState* PRPlayerState = PlayerCharacter->GetPlayerState<APRPlayerState>();
	if (!PRPlayerState)
	{
		return;
	}

	APRGMB* PRGameMode = GetWorld() ? Cast<APRGMB>(GetWorld()->GetAuthGameMode()) : nullptr;
	if (!PRGameMode)
	{
		return;
	}

	const EPRMatchRound CurrentRound = PRGameMode->GetCurrentRound();
	if (CurrentRound != EPRMatchRound::Round1 && CurrentRound != EPRMatchRound::Round2)
	{
		return;
	}

	if (PRPlayerState->IsFinished())
	{
		return;
	}

	PRGameMode->RegisterPlayerFinish(PlayerCharacter, PRPlayerState);

	UE_LOG(LogTemp, Warning, TEXT("[FinishTrigger] %s entered finish trigger / Round: %d"),
		*PRPlayerState->GetPlayerName(),
		static_cast<int32>(CurrentRound));
}