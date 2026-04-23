// PRSoundManager.h
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Sound/SoundBase.h"
#include "PRSoundManager.generated.h"

class UAudioComponent;

UENUM(BlueprintType)
enum class EPRBGMType : uint8
{
    None,
    Title,
    Lobby,
    Round1,
    Round2,
    Result
};

UCLASS()
class PLANTYRACE_API APRSoundManager : public AActor
{
    GENERATED_BODY()

public:
    APRSoundManager();

protected:
    virtual void BeginPlay() override;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio")
    TObjectPtr<USceneComponent> Root;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio")
    TObjectPtr<UAudioComponent> BGMComponent;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio|BGM")
    TObjectPtr<USoundBase> TitleBGM;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio|BGM")
    TObjectPtr<USoundBase> LobbyBGM;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio|BGM")
    TObjectPtr<USoundBase> Round1BGM;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio|BGM")
    TObjectPtr<USoundBase> Round2BGM;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio|BGM")
    TObjectPtr<USoundBase> ResultBGM;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio|SFX")
    TObjectPtr<USoundBase> CheckPointSFX;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio|SFX")
    TObjectPtr<USoundBase> RespawnSFX;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio|SFX")
    TObjectPtr<USoundBase> FinishSFX;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio|SFX")
    TObjectPtr<USoundBase> VictorySFX;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio|SFX")
    TObjectPtr<USoundBase> RoundStartSFX;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Audio")
    float BGMFadeTime = 1.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Audio")
    EPRBGMType CurrentBGMType = EPRBGMType::None;

public:
    // 공용 BGM
    UFUNCTION(BlueprintCallable, Category = "Audio|BGM")
    void PlayBGMByType(EPRBGMType NewBGMType);

    UFUNCTION(BlueprintCallable, Category = "Audio|BGM")
    void StopBGM();

    // 개인별 SFX (호출된 로컬 클라이언트에서만 재생)
    UFUNCTION(BlueprintCallable, Category = "Audio|SFX")
    void PlayCheckPointSFX(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Audio|SFX")
    void PlayRespawnSFX(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Audio|SFX")
    void PlayFinishSFX(const FVector& Location);

    UFUNCTION(BlueprintCallable, Category = "Audio|SFX")
    void PlayVictorySFX();

    UFUNCTION(BlueprintCallable, Category = "Audio|SFX")
    void PlayRoundStartSFX();

    // 공용 BGM만 멀티캐스트
    UFUNCTION(NetMulticast, Reliable)
    void MulticastPlayBGMByType(EPRBGMType NewBGMType);

    UFUNCTION(NetMulticast, Reliable)
    void MulticastStopBGM();

    UFUNCTION(BlueprintPure, Category = "SFX")
    USoundBase* GetFinishSFX() const { return FinishSFX; }

protected:
    USoundBase* GetBGMByType(EPRBGMType InType) const;
};