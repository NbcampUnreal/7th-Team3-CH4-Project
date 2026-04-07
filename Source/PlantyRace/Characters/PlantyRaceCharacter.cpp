#include "PlantyRaceCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Net/UnrealNetwork.h"

APlantyRaceCharacter::APlantyRaceCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;
	GetCharacterMovement()->JumpZVelocity = 700.f;
	
	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
	SpringArmComp->SetupAttachment(RootComponent);
	SpringArmComp->TargetArmLength = 800.f;
	SpringArmComp->SetRelativeRotation(FRotator(-55.f, 0.f, 0.f)); // 위에서 아래로
	SpringArmComp->bUsePawnControlRotation = true;
	
	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	CameraComp->SetupAttachment(SpringArmComp, USpringArmComponent::SocketName);
	CameraComp->bUsePawnControlRotation = false;
	
	PantsSkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("PantsMesh"));
	PantsSkeletalMesh->SetupAttachment(GetMesh());
	PantsSkeletalMesh->SetLeaderPoseComponent(GetMesh());

	ShirtSkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ShirtMesh"));
	ShirtSkeletalMesh->SetupAttachment(GetMesh());
	ShirtSkeletalMesh->SetLeaderPoseComponent(GetMesh());

	HairSkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("HairMesh"));
	HairSkeletalMesh->SetupAttachment(GetMesh());
	HairSkeletalMesh->SetLeaderPoseComponent(GetMesh());
	
	GlassSkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("GlassMesh"));
	GlassSkeletalMesh->SetupAttachment(GetMesh());
	GlassSkeletalMesh->SetLeaderPoseComponent(GetMesh());
	
	ShoeSkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ShoeMesh"));
	ShoeSkeletalMesh->SetupAttachment(GetMesh());
	ShoeSkeletalMesh->SetLeaderPoseComponent(GetMesh());
	
	
	MouseSensitivity =1.5f;
	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)
}




void APlantyRaceCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D LookValue = Value.Get<FVector2D>();


	if (!Controller)
	{
		return;
	}

	AddControllerYawInput(LookValue.X * MouseSensitivity);
	AddControllerPitchInput(-LookValue.Y * MouseSensitivity);
}

void APlantyRaceCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D MoveValue = Value.Get<FVector2D>();

	if (!Controller)
	{
		return;
	}

	const FRotator ControlRot = Controller->GetControlRotation();
	const FRotator YawRot(0.f, ControlRot.Yaw, 0.f);

	const FVector ForwardDir = FRotationMatrix(YawRot).GetUnitAxis(EAxis::X);
	const FVector RightDir = FRotationMatrix(YawRot).GetUnitAxis(EAxis::Y);

	AddMovementInput(ForwardDir, MoveValue.Y);
	AddMovementInput(RightDir, MoveValue.X);
}

void APlantyRaceCharacter::StartJump(const FInputActionValue& Value)
{
	Jump();
}

void APlantyRaceCharacter::EndJump(const FInputActionValue& Value)
{
	StopJumping();
}

void APlantyRaceCharacter::Grab(const FInputActionValue& Value)
{
	ServerGrab();
}


void APlantyRaceCharacter::ServerGrab_Implementation()
{
	FVector Start = GetActorLocation();
	FVector End = Start + GetActorForwardVector() * 200.f;

	FHitResult Hit;
	GetWorld()->SweepSingleByChannel(
		Hit,
		Start,
		End,
		FQuat::Identity,
		ECC_Pawn,
		FCollisionShape::MakeSphere(50.f)
	);

	APlantyRaceCharacter* Target = Cast<APlantyRaceCharacter>(Hit.GetActor());
	if (Target)
	{
		//잡은 타켓이 어떻게 될지는 아직 미정.
	}

}

void APlantyRaceCharacter::ServerRelease_Implementation()
{
	
}

// Called when the game starts or when spawned
void APlantyRaceCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	InitializeModularMeshes();
}

// Called every frame
void APlantyRaceCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void APlantyRaceCharacter::RandomizeClothes()
{
	SetRandomMesh(PantsSkeletalMesh, PantsOptions);
	SetRandomMesh(ShirtSkeletalMesh, ShirtOptions);
	SetRandomMesh(HairSkeletalMesh, HairOptions);
	SetRandomMesh(ShoeSkeletalMesh, ShoeOptions);
}

void APlantyRaceCharacter::InitializeModularMeshes()
{
	ModularMeshes.Empty();

	if (PantsSkeletalMesh)
	{
		PantsSkeletalMesh->SetLeaderPoseComponent(GetMesh());
		ModularMeshes.Add(PantsSkeletalMesh);
	}

	if (ShirtSkeletalMesh)
	{
		ShirtSkeletalMesh->SetLeaderPoseComponent(GetMesh());
		ModularMeshes.Add(ShirtSkeletalMesh);
	}

	if (HairSkeletalMesh)
	{
		HairSkeletalMesh->SetLeaderPoseComponent(GetMesh());
		ModularMeshes.Add(HairSkeletalMesh);
	}

	if (GlassSkeletalMesh)
	{
		GlassSkeletalMesh->SetLeaderPoseComponent(GetMesh());
		ModularMeshes.Add(GlassSkeletalMesh);
	}
	
	if (ShoeSkeletalMesh)
	{
		ShoeSkeletalMesh->SetLeaderPoseComponent(GetMesh());
		ModularMeshes.Add(ShoeSkeletalMesh);
	}
}

void APlantyRaceCharacter::SetRandomMesh(USkeletalMeshComponent* TargetMesh, const TArray<TObjectPtr<USkeletalMesh>>& Options)
{
	if (!TargetMesh || Options.Num() == 0)
	{
		return;
	}

	const int32 RandomIndex = FMath::RandRange(0, Options.Num() - 1);
	TargetMesh->SetSkeletalMesh(Options[RandomIndex]);
}

void APlantyRaceCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(APlantyRaceCharacter, bIsGrabbed);
}

// Called to bind functionality to input
void APlantyRaceCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInput =Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		
		if (MoveAction)
		{
			EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APlantyRaceCharacter::Move);
		}

		if (LookAction)
		{
			EnhancedInput->BindAction(LookAction, ETriggerEvent::Triggered, this, &APlantyRaceCharacter::Look);
		}

		if (JumpAction)
		{
			EnhancedInput->BindAction(JumpAction, ETriggerEvent::Started, this, &APlantyRaceCharacter::StartJump);
			EnhancedInput->BindAction(JumpAction, ETriggerEvent::Completed, this, &APlantyRaceCharacter::EndJump);
		}
		
		if (GrabAction)
		{
			EnhancedInput->BindAction(GrabAction, ETriggerEvent::Triggered, this, &APlantyRaceCharacter::Grab);
		}
		
		if (IA_RandomizeClothes)
		{
			EnhancedInput->BindAction(IA_RandomizeClothes, ETriggerEvent::Started, this, &APlantyRaceCharacter::RandomizeClothes);
		}
	}
}

