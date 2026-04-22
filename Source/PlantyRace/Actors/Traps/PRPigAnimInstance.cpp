#include "PRPigAnimInstance.h"
#include "PRPigCharacter.h"

void UPRPigAnimInstance::NativeInitializeAnimation()
{
    Super::NativeInitializeAnimation();

    OwningPig = Cast<APRPigCharacter>(TryGetPawnOwner());
}

void UPRPigAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
    Super::NativeUpdateAnimation(DeltaSeconds);

    if (!IsValid(OwningPig))
    {
        OwningPig = Cast<APRPigCharacter>(TryGetPawnOwner());
        if (!IsValid(OwningPig))
        {
            return;
        }
    }

    FVector Velocity = OwningPig->GetVelocity();
    Speed = FVector(Velocity.X, Velocity.Y, 0.f).Size();

    const float MoveSpeed = 0.f;
    

    if (bShouldMove)
    {
        if (Speed <= MoveSpeed)
        {
            bShouldMove = false;
        }
    }
    else
    {
        if (Speed > MoveSpeed)
        {
            bShouldMove = true;
        }
    }
}
