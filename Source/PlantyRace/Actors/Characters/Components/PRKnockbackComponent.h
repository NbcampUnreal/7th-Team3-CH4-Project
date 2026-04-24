#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PRKnockbackComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PLANTYRACE_API UPRKnockbackComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UPRKnockbackComponent();

	UFUNCTION(BlueprintCallable)
	void ApplyKnockback(const FVector& LaunchVelocity, float InDownDuration);

	UFUNCTION(BlueprintCallable)
	bool IsKnockedDown() const;

	float GetDownDuration() const { return DownDuration; }

private:
	UPROPERTY(EditAnywhere, Category = "Knockback")
	float DownDuration = 1.f;

	FTimerHandle KnockbackTimerHandle;

	void RecoverFromKnockback();
		
};
