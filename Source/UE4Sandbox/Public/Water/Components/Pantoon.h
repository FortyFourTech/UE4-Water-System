#pragma once

#include "Components/SceneComponent.h"
#include "Pantoon.generated.h"

class IWaterMesh;

UCLASS( ClassGroup=(Water), meta=(BlueprintSpawnableComponent) )
class TINYISLANDUE_API UPantoon : public USceneComponent
{
	GENERATED_BODY()

public:	
	UPantoon();

	// Properties
	UPROPERTY(EditAnywhere)
		bool bProjectOnPolygons; // false saves computations

	UPROPERTY(EditAnywhere)
		bool bDrawDebug = false;

	UPROPERTY()
		FVector CurrentLocation;

	UPROPERTY()
		FVector TargetLocation;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
		FVector LocationOffset;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
		FVector CurrentNormal;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
		FVector TargetNormal;

	UPROPERTY(VisibleInstanceOnly)
		bool bIsOnWater = false;

	UPROPERTY(VisibleInstanceOnly)
		bool bIsProjectedOnWater = false;

	// Functions
	FVector UpdateOffset(IWaterMesh* WaterMesh) { return UpdateOffset(WaterMesh, FTransform::Identity); }

	FVector UpdateOffset(IWaterMesh* WaterMesh, const FTransform& ParentTransform);

protected:
	FVector ProjectOnWater(FVector L1, FVector L2, FVector T1, FVector T2, FVector T3);

	FVector CalcVertexOffset(IWaterMesh* WaterMesh, FVector2D P, float Z);

	FVector GetWaveOffset(IWaterMesh* WaterMesh, FVector P);
};
