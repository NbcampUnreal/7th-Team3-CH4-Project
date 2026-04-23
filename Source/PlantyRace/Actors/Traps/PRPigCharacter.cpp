#include "PRPigCharacter.h"
#include "Components/BoxComponent.h"
#include "Actors/Characters/PlantyRaceCharacter.h"
#include "Actors/Characters/Components/PRKnockbackComponent.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "TimerManager.h"
#include "Navigation/PathFollowingComponent.h"

APRPigCharacter::APRPigCharacter()
{
    HitBox = CreateDefaultSubobject<UBoxComponent>(TEXT("HitBox"));
    HitBox->SetupAttachment(RootComponent);
    HitBox->SetBoxExtent(FVector(60.f, 60.f, 60.f));
    HitBox->SetGenerateOverlapEvents(true);                             
    HitBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);          
    HitBox->SetCollisionObjectType(ECC_WorldDynamic);                   
    HitBox->SetCollisionResponseToAllChannels(ECR_Ignore);              
    HitBox->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);       
    HitBox->OnComponentBeginOverlap.AddDynamic(this, &APRPigCharacter::OnPigHitOverlap);
}

void APRPigCharacter::BeginPlay()
{
    Super::BeginPlay();

    InitialLocation = GetActorLocation();

    if (!HasAuthority())
    {
        return;
    }

    StartRandomMove();
}

void APRPigCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(RandomMoveTimerHandle);
    }

    Super::EndPlay(EndPlayReason);
}

void APRPigCharacter::OnPigHitOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (!HasAuthority())
    {
        return;
    }

    if (!OtherActor || OtherActor == this)
    {
        return;
    }

    APlantyRaceCharacter* Character = Cast<APlantyRaceCharacter>(OtherActor);
    if (!IsValid(Character))
    {
        return;
    }

    if (Character->IsKnockedDown())
    {
        return;
    }

    UPRKnockbackComponent* KBC = Character->GetKnockbackComp();
    if (!IsValid(KBC))
    {
        return;
    }

    ApplyKnockback(Character);
}

FVector APRPigCharacter::CalculateKnockbackVelocity(const APlantyRaceCharacter* TargetCharacter)
{
    if (!TargetCharacter)
    {
        return FVector::ZeroVector;
    }

    FVector PigLocation = GetActorLocation();
    FVector TargetLocation = TargetCharacter->GetActorLocation();
    FVector Dir = TargetLocation - PigLocation;
    Dir.Z = 0.f;

    if (Dir.IsNearlyZero())
    {
        return FVector::ZeroVector;
    }

    Dir = Dir.GetSafeNormal();

    FVector LaunchVelocity = Dir * KnockbackPower;
    LaunchVelocity.Z = KnockbackUpPower;

    return LaunchVelocity;
}

void APRPigCharacter::ApplyKnockback(APlantyRaceCharacter* TargetCharacter)
{
    if (!TargetCharacter)
    {
        return;
    }

    UPRKnockbackComponent* KBC = TargetCharacter->GetKnockbackComp();
    if (!IsValid(KBC))
    {
        return;
    }


    FVector LaunchVelocity = CalculateKnockbackVelocity(TargetCharacter);
    if (LaunchVelocity.IsNearlyZero())
    {
        return;
    }

    KBC->ApplyKnockback(LaunchVelocity, DownDuration);
}

void APRPigCharacter::StartRandomMove()
{
    UWorld* World = GetWorld();
    if (!IsValid(World))
    {
        return;
    }

    if (MoveInterval <= 0.f)
    {
        return;
    }

    const float InitialDelay = FMath::FRandRange(0.f, MoveInterval);;

    World->GetTimerManager().SetTimer(
        RandomMoveTimerHandle,
        this,
        &APRPigCharacter::MoveToRandomLocation,
        MoveInterval,
        true,
        InitialDelay
    );
}

void APRPigCharacter::MoveToRandomLocation()
{
    if (!HasAuthority())
    {
        return;
    }

    AAIController* AIC = Cast<AAIController>(GetController());
    if (!IsValid(AIC))
    {
        return;
    }

    if (AIC->GetMoveStatus() == EPathFollowingStatus::Moving)
    {
        return;
    }
    
    UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());
    if (!IsValid(NavSystem))
    {
        return;
    }

    FNavLocation RandomLocation;

    const bool bFound = NavSystem->GetRandomReachablePointInRadius(InitialLocation, MoveRadius, RandomLocation);
    if (!bFound)
    {
        return;
    }

    AIC->MoveToLocation(RandomLocation.Location);
}
