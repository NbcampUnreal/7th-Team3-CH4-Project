#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "PRPigAnimInstance.generated.h"

class APRPigCharacter;

UCLASS()
class PLANTYRACE_API UPRPigAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(BlueprintReadOnly, Category = "Pig")
	TObjectPtr<APRPigCharacter> OwningPig;

	UPROPERTY(BlueprintReadOnly, Category = "Pig")
	float Speed = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "Pig")
	bool bShouldMove = false;

	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

};
