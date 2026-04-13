#include "Actors/Characters/Components/PRKnockbackComponent.h"
#include "Actors/Characters/PlantyRaceCharacter.h"
#include "TimerManager.h"

UPRKnockbackComponent::UPRKnockbackComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

}

void UPRKnockbackComponent::ApplyKnockback(const FVector& LaunchVelocity, float InDownDuration)
{
	APlantyRaceCharacter* OwnerCharacter = Cast<APlantyRaceCharacter>(GetOwner());
	if (!IsValid(OwnerCharacter))
	{
		return;
	}

	if (OwnerCharacter->IsKnockedDown())
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		return;
	}

	DownDuration = InDownDuration;
	OwnerCharacter->SetKnockedDown(true);
	OwnerCharacter->LaunchCharacter(LaunchVelocity, true, true);
	World->GetTimerManager().ClearTimer(KnockbackTimerHandle);
	World->GetTimerManager().SetTimer(
		KnockbackTimerHandle,
		this,
		&UPRKnockbackComponent::RecoverFromKnockback,
		DownDuration,
		false
	);
}

bool UPRKnockbackComponent::IsKnockedDown() const
{
	const APlantyRaceCharacter* OwnerCharacter = Cast<APlantyRaceCharacter>(GetOwner());


	return IsValid(OwnerCharacter) ? OwnerCharacter->IsKnockedDown() : false;
}

void UPRKnockbackComponent::RecoverFromKnockback()
{
	APlantyRaceCharacter* OwnerCharacter = Cast<APlantyRaceCharacter>(GetOwner());
	if (!IsValid(OwnerCharacter))
	{
		return;
	}

	OwnerCharacter->SetKnockedDown(false);
}


