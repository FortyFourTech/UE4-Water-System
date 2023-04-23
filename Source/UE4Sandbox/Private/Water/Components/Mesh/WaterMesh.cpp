#include "Water/Components/Mesh/WaterMesh.h"

IWaterMesh::IWaterMesh()
{
	TS.Name = FName("WaveScale");
	TS.Association = EMaterialParameterAssociation::GlobalParameter;
	TS.Index = INDEX_NONE;

	BS.Name = FName("WaveBaseScale");
	BS.Association = EMaterialParameterAssociation::GlobalParameter;
	BS.Index = INDEX_NONE;

	WT.Name = FName("WaveTile");
	WT.Association = EMaterialParameterAssociation::GlobalParameter;
	WT.Index = INDEX_NONE;

	WS.Name = FName("WaveSpeed");
	WS.Association = EMaterialParameterAssociation::GlobalParameter;
	WS.Index = INDEX_NONE;
}

bool IWaterMesh::GetMaterialParameters(float &WaveScale, float &WaveBaseScale, float &WaveTile, float &WaveSpeed)
{
    auto* meshComp = Cast<UMeshComponent>(this);
	check(meshComp);

	if (!IsValid(meshComp) || !IsValid(meshComp->GetMaterial(0))) return false;

	if (auto* waterMat = meshComp->GetMaterial(0))
	if (auto* waterMatInst = Cast<UMaterialInstance>(waterMat))
	if (waterMatInst->GetScalarParameterValue(TS, WaveScale) &&
		waterMatInst->GetScalarParameterValue(BS, WaveBaseScale) &&
		waterMatInst->GetScalarParameterValue(WT, WaveTile) &&
		waterMatInst->GetScalarParameterValue(WS, WaveSpeed))
	{
		return true;
	}

	return false;
}

float IWaterMesh::GetWaterZCoord()
{
    auto* thisComp = Cast<USceneComponent>(this);
	check(thisComp);
	return thisComp->GetComponentLocation().Z;
}

FBox IWaterMesh::GetBoundingBox()
{
    auto* thisComp = Cast<USceneComponent>(this);
	check(thisComp);
	return thisComp->Bounds.GetBox();
}

float IWaterMesh::Det(FVector2D V1, FVector2D V2)
{
	return V1.X * V2.Y - V1.Y * V2.X;
}

bool IWaterMesh::IsInsideTris(FVector2D P, FVector2D T1, FVector2D T2, FVector2D T3)
{
	FVector2D v2 = T2 - T1;
	FVector2D v3 = T3 - T1;

	float a = (Det(P, v3) - Det(T1, v3)) / Det(v2, v3);
	float b = -(Det(P, v2) - Det(T1, v2)) / Det(v2, v3);

	return a > 0 && b > 0 && (a + b) <= 1;
}
