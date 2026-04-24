
#include "PRSlidingPlatform.h"
#include "Net/UnrealNetwork.h"

APRSlidingPlatform::APRSlidingPlatform()
{
    PrimaryActorTick.bCanEverTick = true;
    
    bReplicates = true;

    SetNetUpdateFrequency(10.f);

    MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
    RootComponent = MeshComp;

    MeshComp->SetMobility(EComponentMobility::Movable);
    MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    MeshComp->SetCollisionResponseToAllChannels(ECR_Block);
    
    // 기본값 설정
    MaxTiltAngle = 35.0f;
    TiltSpeed = 1.5f;

    // 캐릭터가 미끄러지기 위해서는 표면 마찰력이 낮거나 경사가 커야 함 
    MeshComp->SetMobility(EComponentMobility::Movable);
}


void APRSlidingPlatform::BeginPlay()
{
    Super::BeginPlay();

    NetUpdatePeriod = 1.f / GetNetUpdateFrequency();

    // 시작할 때 현재 회전값 한번 맞춰줌
    const FRotator InitRot = MakeRotationFromElapsedTime(ElapsedTime); 
    SetActorRotation(InitRot); 
}


void APRSlidingPlatform::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (HasAuthority())
    {
        // 서버는 진짜 시간 진행 + 회전 계산 + 복제 기준값 업데이트
        ElapsedTime += DeltaTime * TiltSpeed;
        ServerElapsedTime = ElapsedTime;

        SetActorRotation(MakeRotationFromElapsedTime(ElapsedTime));
    }
    else
    {
        if (NetUpdatePeriod < KINDA_SMALL_NUMBER)
        {
            return;
        }

        // 클라는 마지막 서버값에서 다음 서버값까지 시간값을 보간
        AccDeltaSinceReplicated += DeltaTime;

        const float LerpRatio = FMath::Clamp(
            AccDeltaSinceReplicated / NetUpdatePeriod,
            0.f,
            1.f
        );

        // 서버는 ElapsedTime을 DeltaTime * TiltSpeed 로 증가시키니까
        // 다음 복제 시점 예상값도 NetUpdatePeriod * TiltSpeed 만큼 더해줌
        const float NextEstimatedElapsedTime =
            LatestReplicatedElapsedTime + (NetUpdatePeriod * TiltSpeed);

        const float InterpolatedElapsedTime = FMath::Lerp(
            LatestReplicatedElapsedTime,
            NextEstimatedElapsedTime,
            LerpRatio
        );

        SetActorRotation(MakeRotationFromElapsedTime(InterpolatedElapsedTime));
    }
}

void APRSlidingPlatform::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    DOREPLIFETIME(APRSlidingPlatform, ServerElapsedTime);
}

void APRSlidingPlatform::OnRep_ServerElapsedTime()
{
    PrevReplicatedElapsedTime = LatestReplicatedElapsedTime;
    LatestReplicatedElapsedTime = ServerElapsedTime;
    AccDeltaSinceReplicated = 0.f;
}

FRotator APRSlidingPlatform::MakeRotationFromElapsedTime(float InElapsedTime) const
{
    const float CurrentPitch = FMath::Sin(InElapsedTime) * MaxTiltAngle;
    const float CurrentRoll  = FMath::Cos(InElapsedTime * 0.7f) * MaxTiltAngle;

    return FRotator(CurrentPitch, 0.f, CurrentRoll);
}