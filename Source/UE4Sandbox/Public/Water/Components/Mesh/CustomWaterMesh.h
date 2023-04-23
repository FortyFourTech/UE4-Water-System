#pragma once

#include "Components/StaticMeshComponent.h"
#include "Water/Components/Mesh/WaterMesh.h"
#include <array>
#include <vector>
#include "CustomWaterMesh.generated.h"

UCLASS(ClassGroup=(Water), Meta=(BlueprintSpawnableComponent))
class TINYISLANDUE_API UCustomWaterMesh : public UStaticMeshComponent, public IWaterMesh
{
	GENERATED_BODY()

public:	
	UCustomWaterMesh();

protected:
	virtual void BeginPlay() override;
    
private:
	//Array of triangles each represented by three vertices each of those in turn are 
	//dsecribed as locations (3 floats, packed into FVector)
	std::vector<std::array<FVector, 3>> GatheredGeometry;

public:
    // IWaterMesh
    bool ProjectObjectOnGeometry(const FVector2D& Location,
        FVector2D& v1loc, FVector2D& v2loc, FVector2D& v3loc) override;

    virtual float GetWaterZCoord() override;

private:
	void GatherGeometry();
};
