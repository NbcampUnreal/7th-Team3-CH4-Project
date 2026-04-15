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
	MeshComp->SetNotifyRigidBodyCollision(true);
	MeshComp->OnComponentHit.AddDynamic(this, &APRRotatingActor::OnHit);
}

void APRRotatingActor::BeginPlay()
{
	Super::BeginPlay();

	StartAngle = GetCurrentAxisAngle();
	
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

void APRRotatingActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	if (!MeshComp)
	{
		return;
	}

	MeshComp->SetRelativeLocation(MeshOffset);
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
	const FVector MeshLocation = MeshComp->GetComponentLocation();
	
	FVector RadiusDir = MeshLocation - PivotLocation;
	RadiusDir = PlanarDirection(RadiusDir);
	if (RadiusDir.IsNearlyZero())
	{
		return;
	}

	RadiusDir = RadiusDir.GetSafeNormal();

	FVector TangentDir = TangentDirection(RadiusDir);
	if (TangentDir.IsNearlyZero())
	{
		return;
	}

	FVector ToCharacter = HitCharacter->GetActorLocation() - MeshLocation;
	ToCharacter = PlanarDirection(ToCharacter);
	if (ToCharacter.IsNearlyZero())
	{
		return;
	}

	ToCharacter = ToCharacter.GetSafeNormal();

	const float SwingDot = FVector::DotProduct(TangentDir, ToCharacter);
	if (SwingDot >= 0.f)
	{
		return;
	}
	
	const FVector LaunchVelocity = (TangentDir * KnockbackPower) + FVector(0.f, 0.f, KnockbackUpPower);

	KBC->ApplyKnockback(LaunchVelocity, KnockbackDownDuration);
}

float APRRotatingActor::GetCurrentAxisAngle() const
{
	const FRotator RelativeRot = Scene->GetRelativeRotation();

	switch (RotationAxis)
	{
	case ERotationAxis::X:
		return RelativeRot.Roll;
	case ERotationAxis::Y:
		return RelativeRot.Pitch;
	case ERotationAxis::Z:
		return RelativeRot.Yaw;
	default:
		return 0.f;
	}
}

FRotator APRRotatingActor::RotationFromAngle(float Angle) const
{
	switch (RotationAxis)
	{
	case ERotationAxis::X:
		return FRotator(0.f, 0.f, Angle);
	case ERotationAxis::Y:
		return FRotator(Angle, 0.f, 0.f);
	case ERotationAxis::Z:
		return FRotator(0.f, Angle, 0.f);
	default:
		return FRotator::ZeroRotator;
	}
}

FVector APRRotatingActor::PlanarDirection(const FVector& Source) const
{
	FVector Result = Source;

	switch (RotationAxis)
	{
	case ERotationAxis::X:
		Result.X = 0.f;
		break;
	case ERotationAxis::Y:
		Result.Y = 0.f;
		break;
	case ERotationAxis::Z:
		Result.Z = 0.f;
		break;
	default:
		break;
	}

	return Result;
}

FVector APRRotatingActor::TangentDirection(const FVector& RadiusDir) const
{
	FVector TangentDir = FVector::ZeroVector;

	switch (RotationAxis)
	{
	case ERotationAxis::X:
		TangentDir = FVector(0.f, -RadiusDir.Z, RadiusDir.Y);
		break;
	case ERotationAxis::Y:
		TangentDir = FVector(RadiusDir.Z, 0.f, -RadiusDir.X);
		break;
	case ERotationAxis::Z:
		TangentDir = FVector(-RadiusDir.Y, RadiusDir.X, 0.f);
		break;
	default:
		break;
	}

	TangentDir = TangentDir.GetSafeNormal();

	if (DegreesPerSecond < 0.f)
	{
		TangentDir *= -1.f;
	}

	return TangentDir;
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
	const float NewAngle = StartAngle + ElapsedTime * DegreesPerSecond;
	Scene->SetRelativeRotation(RotationFromAngle(NewAngle));
}
