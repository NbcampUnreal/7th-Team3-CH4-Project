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
		ForceNetUpdate();
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

	const FVector KnockbackDir = Hit.Normal * -1.f;
	const FVector LaunchVelocity = (KnockbackDir * KnockbackPower) + FVector(0.f, 0.f, KnockbackUpPower);
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
