#pragma once

#include "WaterMesh.generated.h"

UINTERFACE()
class TINYISLANDUE_API UWaterMesh : public UInterface
{
    GENERATED_BODY()
};

class TINYISLANDUE_API IWaterMesh
{    
    GENERATED_BODY()

public:
    IWaterMesh();

private:
	FMaterialParameterInfo TS, BS, WT, WS;
    
public:
    virtual bool ProjectObjectOnGeometry(const FVector2D& Location,
        FVector2D& v1loc, FVector2D& v2loc, FVector2D& v3loc) = 0;

    bool GetMaterialParameters(float& WaveScale, float& WaveBaseScale,
        float& WaveTile, float& WaveSpeed);

    virtual float GetWaterZCoord();
    FBox GetBoundingBox();

protected:
	float Det(FVector2D V1, FVector2D V2);
	bool IsInsideTris(FVector2D P, FVector2D T1, FVector2D T2, FVector2D T3);
};