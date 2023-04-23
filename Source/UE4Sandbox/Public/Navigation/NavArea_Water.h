#pragma once

#include "NavAreas/NavArea.h"

#include "NavArea_Water.generated.h"

UCLASS(BlueprintType)
class TINYISLANDUE_API UNavArea_Water : public UNavArea
{
    GENERATED_BODY()

public:
    UNavArea_Water(const FObjectInitializer& ObjectInitializer);
};
