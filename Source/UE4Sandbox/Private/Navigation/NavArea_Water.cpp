#include "Navigation/NavArea_Water.h"

UNavArea_Water::UNavArea_Water(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
	DefaultCost = 1000.f;
	FixedAreaEnteringCost = 500.f;
	DrawColor = FColor::Blue;
}
