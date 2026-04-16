// FinishTrigger.cpp
#include "GameMode/FinishTrigger.h"
#include "Components/SceneComponent.h"
#include "Components/BoxComponent.h"
#include "Actors/Characters/PlantyRaceCharacter.h"
#include "Core/PRPlayerState.h"
#include "GameMode/PRGMB.h"
#include "GameFramework/GameModeBase.h"
#include "Core/PRGameStateBase.h"
#include "Audio/PRSoundManager.h"
#include "Engine/World.h"

AFinishTrigger::AFinishTrigger()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	TriggerBox->SetupAttachment(SceneRoot);
	TriggerBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	TriggerBox->SetBoxExtent(FVector(200.f, 500.f, 300.f));
}

void AFinishTrigger::BeginPlay()
{
	Super::BeginPlay();

	if (TriggerBox)
	{
		TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &AFinishTrigger::OnFinishTriggerBeginOverlap);
	}
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

	APRGameStateBase* GS = GetWorld()->GetGameState<APRGameStateBase>();
	if (IsValid(GS))
	{
		if (APRSoundManager* SM = GS->GetSoundManager())
		{
			SM->PlayFinishSFX(GetActorLocation());
		}
	}

	PRGameMode->RegisterPlayerFinish(PlayerCharacter, PRPlayerState);

	UE_LOG(LogTemp, Warning, TEXT("[FinishTrigger] %s entered finish trigger"),
		*PRPlayerState->GetPlayerName());
}