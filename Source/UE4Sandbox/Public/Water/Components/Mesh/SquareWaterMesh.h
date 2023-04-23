#pragma once

#include "Components/InstancedStaticMeshComponent.h"
#include "Water/Components/Mesh/WaterMesh.h"
#include "SquareWaterMesh.generated.h"

UCLASS(ClassGroup=(Water), Meta=(BlueprintSpawnableComponent))
class TINYISLANDUE_API USquareWaterMesh : public UInstancedStaticMeshComponent, public IWaterMesh
{
	GENERATED_BODY()

public:	
	USquareWaterMesh();

#if WITH_EDITOR
    void PostEditChangeProperty(struct FPropertyChangedEvent& e) override;
#endif // WITH_EDITOR

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Parameters")
    int32 Radius;
    
    // IWaterMesh
    bool ProjectObjectOnGeometry(const FVector2D& Location,
        FVector2D& v1loc, FVector2D& v2loc, FVector2D& v3loc) override;

private:
    bool IsIndexInsideRadius(int32 XIdx, int32 YIdx);
};
