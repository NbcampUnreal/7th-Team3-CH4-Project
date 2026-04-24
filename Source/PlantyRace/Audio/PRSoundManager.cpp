// PRSoundManager.cpp
#include "Audio/PRSoundManager.h"
#include "Components/AudioComponent.h"
#include "Components/SceneComponent.h"
#include "Kismet/GameplayStatics.h"

APRSoundManager::APRSoundManager()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;

    Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    SetRootComponent(Root);

    BGMComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("BGMComponent"));
    BGMComponent->SetupAttachment(Root);
    BGMComponent->bAutoActivate = false;
    BGMComponent->bIsUISound = true;
    BGMComponent->bAllowSpatialization = false;
    
    bAlwaysRelevant = true;
}

void APRSoundManager::BeginPlay()
{
    Super::BeginPlay();
}

USoundBase* APRSoundManager::GetBGMByType(EPRBGMType InType) const
{
    switch (InType)
    {
    case EPRBGMType::Title:
        return TitleBGM;
    case EPRBGMType::Lobby:
        return LobbyBGM;
    case EPRBGMType::Round1:
        return Round1BGM;
    case EPRBGMType::Round2:
        return Round2BGM;
    case EPRBGMType::Result:
        return ResultBGM;
    default:
        return nullptr;
    }
}

void APRSoundManager::PlayBGMByType(EPRBGMType NewBGMType)
{
    if (HasAuthority())
    {
        MulticastPlayBGMByType(NewBGMType);
        return;
    }

    if (CurrentBGMType == NewBGMType)
    {
        return;
    }

    USoundBase* NewBGM = GetBGMByType(NewBGMType);
    if (!IsValid(NewBGM) || !IsValid(BGMComponent))
    {
        return;
    }

    if (BGMComponent->IsPlaying())
    {
        BGMComponent->FadeOut(BGMFadeTime, 0.0f);
    }

    BGMComponent->SetSound(NewBGM);
    BGMComponent->FadeIn(BGMFadeTime, 1.0f, 0.0f);

    CurrentBGMType = NewBGMType;
}

void APRSoundManager::StopBGM()
{
    if (HasAuthority())
    {
        MulticastStopBGM();
        UE_LOG(LogTemp, Error, TEXT("사운드 중단점 1 "));

        return;
    }

    if (!IsValid(BGMComponent))
    {
        return;
    }

    if (BGMComponent->IsPlaying())
    {
        BGMComponent->FadeOut(BGMFadeTime, 0.0f);
    }

    CurrentBGMType = EPRBGMType::None;
}

void APRSoundManager::PlayCheckPointSFX(const FVector& Location)
{
    if (!IsValid(CheckPointSFX))
    {
        return;
    }

    UGameplayStatics::PlaySoundAtLocation(this, CheckPointSFX, Location);
}

void APRSoundManager::PlayRespawnSFX(const FVector& Location)
{
    if (!IsValid(RespawnSFX))
    {
        return;
    }

    UGameplayStatics::PlaySoundAtLocation(this, RespawnSFX, Location);
}

void APRSoundManager::PlayFinishSFX(const FVector& Location)
{
    if (!IsValid(FinishSFX))
    {
        UE_LOG(LogTemp, Error, TEXT("[FinishSFX] FinishSFX null"));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("[FinishSFX] PlaySound2D 실행"));
    UGameplayStatics::PlaySound2D(this, FinishSFX);
}

void APRSoundManager::PlayVictorySFX()
{
    if (!IsValid(VictorySFX))
    {
        return;
    }

    UGameplayStatics::PlaySound2D(this, VictorySFX);
}

void APRSoundManager::PlayRoundStartSFX()
{
    if (!IsValid(RoundStartSFX))
    {
        return;
    }

    UGameplayStatics::PlaySound2D(this, RoundStartSFX);
}

void APRSoundManager::MulticastPlayBGMByType_Implementation(EPRBGMType NewBGMType)
{
    if (CurrentBGMType == NewBGMType)
    {
        return;
    }

    USoundBase* NewBGM = GetBGMByType(NewBGMType);
    if (!IsValid(NewBGM) || !IsValid(BGMComponent))
    {
        return;
    }

    if (BGMComponent->IsPlaying())
    {
        BGMComponent->FadeOut(BGMFadeTime, 0.0f);
    }

    BGMComponent->SetSound(NewBGM);
    BGMComponent->FadeIn(BGMFadeTime, 1.0f, 0.0f);

    CurrentBGMType = NewBGMType;
}

void APRSoundManager::MulticastStopBGM_Implementation()
{
    if (!IsValid(BGMComponent))
    {
        UE_LOG(LogTemp, Error, TEXT("사운드 중단점 2 "));

        return;
    }

    if (BGMComponent->IsPlaying())
    {
        BGMComponent->FadeOut(BGMFadeTime, 0.0f);
    }

    CurrentBGMType = EPRBGMType::None;
}