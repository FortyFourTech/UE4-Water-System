#pragma once

#include "ProceduralMeshComponent.h"
#include "Water/Components/Mesh/WaterMesh.h"
#include "CircleSegmentWaterMesh.generated.h"

UCLASS(ClassGroup=(Water), Meta=(BlueprintSpawnableComponent))
class TINYISLANDUE_API UCircleSegmentWaterMesh : public UProceduralMeshComponent, public IWaterMesh
{
	GENERATED_BODY()

public:	
	UCircleSegmentWaterMesh(const FObjectInitializer& ObjectInitializer);

#if WITH_EDITOR
    void PostEditChangeProperty(struct FPropertyChangedEvent& e) override;
#endif // WITH_EDITOR

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Parameters")
    float MinRadius = 10.f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Parameters")
    int32 RingsNum = 50;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Parameters")
    float SegmentAngle = 60.f;
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Parameters")
    float EdgeAngle = 2.f;
    
    // IWaterMesh
    bool ProjectObjectOnGeometry(const FVector2D& Location,
        FVector2D& v1loc, FVector2D& v2loc, FVector2D& v3loc) override;

private:
    void RebuildMesh();
	FVector2D RadiusSearch(const TArray<float>& Cache, float Value);
};
