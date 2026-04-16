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

	SetLifeSpan(LifeSeconds);
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

	UPRKnockbackComponent* KBC = Character->GetKnockbackComp();
	if (!IsValid(KBC))
	{
		return;
	}

	FVector KnockbackDir = Character->GetActorLocation() - Hit.ImpactPoint;
	
	FVector AdjustedDir = FVector(KnockbackDir.X, KnockbackDir.Y, 0.3f);
	AdjustedDir = AdjustedDir.GetSafeNormal();

	FVector LaunchVelocity = AdjustedDir * KnockbackPower;

	KBC->ApplyKnockback(LaunchVelocity, KnockdownTime);
}

void APRRollingActor::ApplyStartImpulse()
{
	if (!MeshComp)
	{
		return;
	}

	FVector InitialDir = GetInitialRollDirection();

	float StartImpulseStrength = FMath::RandRange(MinStartImpulse, MaxStartImpulse);

	FVector Impulse = InitialDir * StartImpulseStrength;

	MeshComp->AddImpulse(Impulse);
}

FVector APRRollingActor::GetInitialRollDirection() const
{
	FVector ForwardDir = GetActorForwardVector();

	FVector FlatDir = FVector(ForwardDir.X, ForwardDir.Y, 0.f);

	return FlatDir.GetSafeNormal();
}

