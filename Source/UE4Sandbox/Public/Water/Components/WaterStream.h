#pragma once

#include "Components/SplineComponent.h"

#include "WaterStream.generated.h"

UCLASS(ClassGroup=(Water), Meta=(BlueprintSpawnableComponent))
class TINYISLANDUE_API UWaterStream : public USplineComponent
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Parameters")
    float InfluenceDistance = 200.f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Parameters")
    float Force = 1.f;
};
