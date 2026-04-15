#include "PRJumpPad.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Actors/Characters/PlantyRaceCharacter.h"

APRJumpPad::APRJumpPad()
{
	PrimaryActorTick.bCanEverTick = false;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	SetRootComponent(MeshComp);

	BoxComp = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComp"));
	BoxComp->SetupAttachment(MeshComp);
	BoxComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	BoxComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	BoxComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	BoxComp->SetGenerateOverlapEvents(true);
	BoxComp->OnComponentBeginOverlap.AddDynamic(this, &APRJumpPad::OnBeginOverlap);
}

void APRJumpPad::OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	APlantyRaceCharacter* Character = Cast<APlantyRaceCharacter>(OtherActor);
	if (!IsValid(Character))
	{
		return;
	}

	const FVector LaunchVelocity = GetLaunchVelocity();

	Character->LaunchCharacter(LaunchVelocity, true, true);
}

FVector APRJumpPad::GetLaunchVelocity() const
{
	const FVector UpVelocity = GetActorUpVector() * LaunchUpPower;
	if (JumpMode == EJumpPadMode::Z)
	{
		return UpVelocity;
	}

	const FVector ForwardVelocity = GetActorForwardVector() * LaunchForwardPower;

	return UpVelocity + ForwardVelocity;
}


