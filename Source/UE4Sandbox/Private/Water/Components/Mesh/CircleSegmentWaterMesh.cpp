#include "Water/Components/Mesh/CircleSegmentWaterMesh.h"

UCircleSegmentWaterMesh::UCircleSegmentWaterMesh(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    bUseComplexAsSimpleCollision = true;

    SetCollisionProfileName("NoCollision", false);
    SetGenerateOverlapEvents(false);
}

#if WITH_EDITOR
void UCircleSegmentWaterMesh::PostEditChangeProperty(FPropertyChangedEvent& e)
{
    FName changedPropertyName = (e.Property != NULL) ? e.Property->GetFName() : NAME_None;

	if (changedPropertyName == GET_MEMBER_NAME_CHECKED(UCircleSegmentWaterMesh, MinRadius) ||
	    changedPropertyName == GET_MEMBER_NAME_CHECKED(UCircleSegmentWaterMesh, RingsNum) ||
	    changedPropertyName == GET_MEMBER_NAME_CHECKED(UCircleSegmentWaterMesh, SegmentAngle) ||
	    changedPropertyName == GET_MEMBER_NAME_CHECKED(UCircleSegmentWaterMesh, EdgeAngle))
    {
        RebuildMesh();
    }

    Super::PostEditChangeProperty(e);
}
#endif // WITH_EDITOR

bool UCircleSegmentWaterMesh::ProjectObjectOnGeometry(const FVector2D& Location,
    FVector2D& v1loc, FVector2D& v2loc, FVector2D& v3loc)
{
    FVector2D meshOffset = FVector2D(GetComponentLocation());
    FQuat meshRot = GetComponentQuat();
	float meshScale = GetComponentScale().X; // scale must be uniform along XY plane

	auto& radCache = RadialCache::GetRadialCache(MinRadius * meshScale, EdgeAngle, RingsNum);

    FVector2D locationRel = Location - meshOffset;
    locationRel = FVector2D(meshRot.UnrotateVector(FVector(locationRel, 0.f)));
    auto radiuses = RadiusSearch(radCache, FMath::Abs(locationRel.Size()));

    auto r1 = radiuses.X;
    auto r2 = radiuses.Y;

    float angle1 = FMath::Atan2(radiuses.X * locationRel.Y, radiuses.X * locationRel.X);
    angle1 = FMath::RadiansToDegrees(angle1);

    float angle1Idx = angle1 / EdgeAngle;
    float edgeNum = SegmentAngle / EdgeAngle;
        
    if (FMath::Abs(angle1Idx) > (edgeNum / 2.f))
        return false;

    angle1 = FMath::CeilToFloat(angle1Idx) * EdgeAngle;
    float angle2 = FMath::FloorToFloat(angle1Idx) * EdgeAngle;

    angle1 = FMath::DegreesToRadians(angle1);
    angle2 = FMath::DegreesToRadians(angle2);

    auto makeVert = [&] (float angle, float radius) -> FVector2D { return FVector2D(meshRot.RotateVector(FVector(FMath::Cos(angle) * radius, FMath::Sin(angle) * radius, 0.f))) + meshOffset; };
    auto vert1 = makeVert(angle1, r1);
    auto vert2 = makeVert(angle1, r2);
    auto vert3 = makeVert(angle2, r1);
    auto vert4 = makeVert(angle2, r2);

    if (IsInsideTris(Location, vert1, vert2, vert3))
    {
        v1loc = vert1;
        v2loc = vert2;
        v3loc = vert3;
        return true;
    }
    else if (IsInsideTris(Location, vert2, vert3, vert4))
    {
        v1loc = vert2;
        v2loc = vert3;
        v3loc = vert4;
        return true;
    }

    return false;
}

void UCircleSegmentWaterMesh::RebuildMesh()
{
	TArray<FVector> Vertices;
	TArray<int32> Triangles;

    float r1 = MinRadius;
    float r2 = r1;
    float edgeRadAngle2 = FMath::DegreesToRadians(EdgeAngle) / 2.f;

	TArray<FVector> prevRow;

    // create first vertices row
    for (float angleOffset = -(SegmentAngle / 2.f); angleOffset <= (SegmentAngle / 2.f); angleOffset += EdgeAngle)
    {
        float vX;
        float vY;
        FMath::PolarToCartesian(r1, FMath::DegreesToRadians(angleOffset), vX, vY);
        prevRow.Add(FVector(vX, vY, 0.f));
    }
    
    Vertices.Append(prevRow);
    int colsNum = prevRow.Num();
    prevRow.Empty();

    for (int32 itIdx = 0; itIdx < RingsNum; ++itIdx)
    {
        float dr = (2 * r1 * FMath::Sin(edgeRadAngle2)) / (FMath::Cos(edgeRadAngle2) - FMath::Sin(edgeRadAngle2)) ;
        r2 = r1 + dr;
        int vertNum = Vertices.Num();

        for (int colIdx = 0; colIdx < colsNum; ++colIdx)
        {
            float angleOffset = -(SegmentAngle / 2.f) + EdgeAngle * (float)colIdx;
            angleOffset = FMath::DegreesToRadians(angleOffset);

            float vX, vY;
            FMath::PolarToCartesian(r2, angleOffset, vX, vY);
            prevRow.Add(FVector(vX, vY, 0.f));

            if (colIdx > 0)
            {
                Triangles.Add(vertNum + colIdx - colsNum);
                Triangles.Add(vertNum + colIdx);
                Triangles.Add(vertNum + colIdx - 1 - colsNum);

                Triangles.Add(vertNum + colIdx - 1 - colsNum);
                Triangles.Add(vertNum + colIdx);
                Triangles.Add(vertNum + colIdx - 1);
            }
        }
            
        Vertices.Append(prevRow);

        prevRow.Empty();
        r1 = r2;
    }

    CreateMeshSection(0, Vertices, Triangles,
        TArray<FVector>(), // Normals
        TArray<FVector2D>(), // UV0
        TArray<FColor>(), // VertexColors
        TArray<FProcMeshTangent>(), // Tangents
        true // bCreateCollision
    );
}

FVector2D UCircleSegmentWaterMesh::RadiusSearch(const TArray<float>& Cache, float Value)
{
	int cacheLo = 0, cacheHi = Cache.Num();
	int lo = cacheLo, hi = cacheHi;
	int mid;

	while (hi - lo > 1)
	{
		mid = (hi + lo) / 2;
		if (Cache[mid] < Value)
		{
			lo = mid + 1;
		}
		else
		{
			hi = mid;
		}
	}

	if (hi > cacheHi - 1)
	{
		return FVector2D(Cache[cacheHi - 2], Cache[cacheHi - 1]);
	}
	
	if (lo < cacheLo + 1)
	{
		return FVector2D(Cache[cacheLo + 1], Cache[cacheLo + 2]);
	}

	if (hi == lo)
	{
		lo--;
	}

	if (Value < Cache[lo])
	{
		lo--;
		hi--;
	}

	return FVector2D(Cache[lo], Cache[hi]);
}
