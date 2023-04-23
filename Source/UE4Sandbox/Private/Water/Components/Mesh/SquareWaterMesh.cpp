#include "Water/Components/Mesh/SquareWaterMesh.h"

USquareWaterMesh::USquareWaterMesh() : Super()
{
    SetCollisionProfileName("NoCollision", false);
    SetGenerateOverlapEvents(false);
}

#if WITH_EDITOR
void USquareWaterMesh::PostEditChangeProperty(FPropertyChangedEvent& e)
{
    FName changedPropertyName = (e.Property != NULL) ? e.Property->GetFName() : NAME_None;

	if (changedPropertyName == GET_MEMBER_NAME_CHECKED(USquareWaterMesh, Radius) ||
	    changedPropertyName == USquareWaterMesh::GetMemberNameChecked_StaticMesh())
    {
        if (auto* mesh = GetStaticMesh())
        {
            const float instanceSize = mesh->GetBounds().BoxExtent.X * 2.f;

            ClearInstances();
            for (int32 x = Radius * -1; x <= Radius; ++x)
            {
                for (int32 y = Radius * -1; y <= Radius; ++y)
                {
                    if (IsIndexInsideRadius(x, y))
                    {
                        FTransform instanceRelTr (FVector(instanceSize * x, instanceSize * y, 0.f));
                        AddInstance(instanceRelTr);
                    }
                }
            }
        }
    }

    Super::PostEditChangeProperty(e);
}
#endif // WITH_EDITOR

bool USquareWaterMesh::ProjectObjectOnGeometry(const FVector2D& Location,
    FVector2D& v1loc, FVector2D& v2loc, FVector2D& v3loc)
{
    if (Radius < 1) return false;

    // FVector2D waterLoc = GetOwner()->GetActorLocation();

    FVector2D meshOffset = FVector2D(GetComponentLocation());
    FQuat meshRot = GetComponentQuat();
	float meshScale = GetComponentScale().X; // scale must be uniform along XY plane

    FVector2D meshDivs(4.f, 4.f);
    FVector2D meshSize(100.f, 100.f);

    FVector2D quadSize = meshSize * meshScale / meshDivs;

    FVector2D locationCoord = Location - meshOffset;
    locationCoord = FVector2D(meshRot.UnrotateVector(FVector(locationCoord, 0.f)));
    locationCoord = locationCoord / quadSize;
    
    if (!IsIndexInsideRadius(
        FMath::CeilToInt((FMath::Abs(locationCoord.X) - 2.f) / 4.f),
        FMath::CeilToInt((FMath::Abs(locationCoord.Y) - 2.f) / 4.f)))
        return false;

    auto makeVert = [&] (float coordX, float coordY) -> FVector2D { return FVector2D(meshRot.RotateVector(FVector(coordX * quadSize.X, coordY * quadSize.Y, 0.f))) + meshOffset; };
    auto vert1 = makeVert(FMath::CeilToFloat(locationCoord.X), FMath::FloorToFloat(locationCoord.Y));
    auto vert2 = makeVert(FMath::CeilToFloat(locationCoord.X), FMath::CeilToFloat(locationCoord.Y));
    auto vert3 = makeVert(FMath::FloorToFloat(locationCoord.X), FMath::CeilToFloat(locationCoord.Y));
    auto vert4 = makeVert(FMath::FloorToFloat(locationCoord.X), FMath::FloorToFloat(locationCoord.Y));

    if (IsInsideTris(Location, vert1, vert2, vert3))
    {
        v1loc = vert1;
        v2loc = vert2;
        v3loc = vert3;
        return true;
    }
    else if (IsInsideTris(Location, vert1, vert3, vert4))
    {
        v1loc = vert1;
        v2loc = vert3;
        v3loc = vert4;
        return true;
    }

    return false;
}

bool USquareWaterMesh::IsIndexInsideRadius(int32 XIdx, int32 YIdx)
{
    return ((XIdx * XIdx) + (YIdx * YIdx)) <= (Radius * Radius);
}
