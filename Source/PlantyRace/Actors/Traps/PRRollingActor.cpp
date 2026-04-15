#include "PRRollingActor.h"
#include "Components/StaticMeshComponent.h"
#include "Actors/Characters/PlantyRaceCharacter.h"
#include "Actors/Characters/Components/PRKnockbackComponent.h"

APRRollingActor::APRRollingActor()
{
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;
	SetReplicateMovement(true);

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	SetRootComponent(MeshComp);
	MeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	MeshComp->SetNotifyRigidBodyCollision(true);
	MeshComp->SetSimulatePhysics(true);
	MeshComp->SetEnableGravity(true);
	MeshComp->SetLinearDamping(0.2f);
	MeshComp->SetAngularDamping(0.1f);
	MeshComp->OnComponentHit.AddDynamic(this, &APRRollingActor::OnActorHit);
}

void APRRollingActor::BeginPlay()
{
	Super::BeginPlay();

	if (!HasAuthority())
	{
		return;
	}

	ApplyStartImpulse();
}

void APRRollingActor::OnActorHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
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

	UPRKnockbackComponent* KBC = Cast<UPRKnockbackComponent>(Character->GetKnockbackComp());
	if (!IsValid(KBC))
	{
		return;
	}


}

void APRRollingActor::ApplyStartImpulse()
{
	if (!MeshComp)
	{
		return;
	}

	FVector InitialDir = MeshComp->GetForwardVector();

	float StartImpulseStrength = 0.f;

}

FVector APRRollingActor::GetInitialRollDirection() const
{


	return FVector();
}

