#include "Actors/Traps/PRRotatingActor.h"
#include "Core/PRGameStateBase.h"
#include "Components/SceneComponent.h"
#include "Components/ArrowComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Actors/Characters/PlantyRaceCharacter.h"
#include "Actors/Characters/Components/PRKnockbackComponent.h"
#include "Net/UnrealNetwork.h"

APRRotatingActor::APRRotatingActor()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(false);

	Scene = CreateDefaultSubobject<USceneComponent>(TEXT("Scene"));
	SetRootComponent(Scene);

	SceneArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("SceneArrow"));
	SceneArrow->SetupAttachment(Scene);

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetupAttachment(Scene);
	MeshComp->SetRelativeLocation(SceneOffset);
	MeshComp->SetNotifyRigidBodyCollision(true);
	MeshComp->OnComponentHit.AddDynamic(this, &APRRotatingActor::OnHit);

	DegreesPerSecond = 45.f;
	StartYaw = 0.f; 
	ServerStartTime = -1.f;
}

void APRRotatingActor::BeginPlay()
{
	Super::BeginPlay();

	StartYaw = GetActorRotation().Yaw;
	
	if (HasAuthority())
	{
		APRGameStateBase* GS = GetWorld() ? GetWorld()->GetGameState<APRGameStateBase>() : nullptr;
		if (!IsValid(GS))
		{
			return;
		}

		ServerStartTime = GS->GetServerWorldTimeSeconds();
		bCanRotate = true;
	}
}

void APRRotatingActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bCanRotate)
	{
		return;
	}

	UpdateRotationFromServer();
}

void APRRotatingActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, ServerStartTime);
}

void APRRotatingActor::OnRep_ServerStartTime()
{
	UpdateRotationFromServer();

	bCanRotate = true;
}

void APRRotatingActor::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (!HasAuthority())
	{
		return;
	}

	if (!OtherActor || OtherActor == this)
	{
		return;
	}

	APlantyRaceCharacter* HitCharacter = Cast<APlantyRaceCharacter>(OtherActor);
	if (!HitCharacter)
	{
		return;
	}

	UPRKnockbackComponent* KBC = HitCharacter->GetKnockbackComp();
	if (!KBC)
	{
		return;
	}
	
	const FVector PivotLocation = GetActorLocation();
	const FVector HitPoint = Hit.ImpactPoint;
	
	FVector RadiusDir = HitPoint - PivotLocation;
	RadiusDir.Z = 0.f;

	if (RadiusDir.IsNearlyZero())
	{
		return;
	}

	RadiusDir = RadiusDir.GetSafeNormal();

	FVector TangentDir = FVector::ZeroVector;
	if (DegreesPerSecond >= 0.f)
	{
		TangentDir = FVector(-RadiusDir.Y, RadiusDir.X, 0.f);
	}
	else
	{
		TangentDir = FVector(RadiusDir.Y, -RadiusDir.X, 0.f);
	}

	FVector ToImpactDir = HitPoint - MeshComp->GetComponentLocation();
	ToImpactDir.Z = 0.f;

	if (ToImpactDir.IsNearlyZero())
	{
		return;
	}

	ToImpactDir = ToImpactDir.GetSafeNormal();

	const float Dot = FVector::DotProduct(TangentDir, ToImpactDir);
	
	if (Dot <= 0.f)
	{
		return;
	}
	
	const FVector LaunchVelocity = (TangentDir * KnockbackPower) + FVector(0.f, 0.f, KnockbackUpPower);
	const float DownDuration = KnockbackDownDuration;

	KBC->ApplyKnockback(LaunchVelocity, DownDuration);
}

void APRRotatingActor::UpdateRotationFromServer()
{
	APRGameStateBase* GS = GetWorld() ? GetWorld()->GetGameState<APRGameStateBase>() : nullptr;
	if (!IsValid(GS))
	{
		return;
	}

	const float CurrentServerTime = GS->GetServerWorldTimeSeconds();
	const float ElapsedTime = CurrentServerTime - ServerStartTime;
	const float NewYaw = StartYaw + ElapsedTime * DegreesPerSecond;
	Scene->SetRelativeRotation(FRotator(0.f, NewYaw, 0.f));
}
