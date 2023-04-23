#include "Water/Components/Mesh/CustomWaterMesh.h"

UCustomWaterMesh::UCustomWaterMesh() : Super()
{
    SetCollisionProfileName("NoCollision", false);
    SetGenerateOverlapEvents(false);
}

void UCustomWaterMesh::BeginPlay()
{
	Super::BeginPlay();

	GatherGeometry();
}

bool UCustomWaterMesh::ProjectObjectOnGeometry(const FVector2D& Location,
    FVector2D& v1loc, FVector2D& v2loc, FVector2D& v3loc)
{
	FVector2D meshOffset = FVector2D(GetComponentLocation());
    FQuat meshRot = GetComponentQuat();
	float meshScale = GetComponentScale().X; // scale must be uniform along XY plane

	for (int i = 0; i < GatheredGeometry.size(); ++i)
	{
		auto tri3D = GatheredGeometry.at(i);
		FVector2D tri[3];
	    auto makeVert = [&] (FVector vertPos) -> FVector2D { return FVector2D(meshRot.RotateVector(vertPos * meshScale)) + meshOffset; };
		tri[0] = makeVert(tri3D[0]);
		tri[1] = makeVert(tri3D[1]);
		tri[2] = makeVert(tri3D[2]);

		//UE_LOG(LogTemp, Log, TEXT("Tri : {%s}, {%s}, {%s}"), *tri[0].ToString(), *tri[1].ToString(), *tri[2].ToString());

		if (IsInsideTris(Location, tri[0], tri[1], tri[2]))
		{
			v1loc = tri[0];
			v2loc = tri[1];
			v3loc = tri[2];
			return true;
		}
	}

	return false;
}

float UCustomWaterMesh::GetWaterZCoord()
{
	float baseZ = IWaterMesh::GetWaterZCoord();
	float vertexZ = 0.f;

	if (GatheredGeometry.size() > 0)
	{
		vertexZ = GatheredGeometry[0][0].Z;
	}

	return baseZ + vertexZ;
}

void UCustomWaterMesh::GatherGeometry()
{
	if (!IsValidLowLevel()) return;

	UStaticMesh* Mesh = GetStaticMesh();
	if (!Mesh || !Mesh->GetRenderData()) return;

	GatheredGeometry.clear();

	FVector v1pos, v2pos, v3pos;
	auto* meshRenderData = Mesh->GetRenderData();
	if (meshRenderData->LODResources.Num() > 0)
	{
		const auto& meshLod0 = meshRenderData->LODResources[0];
		const FPositionVertexBuffer* VertexBuffer = &meshLod0.VertexBuffers.PositionVertexBuffer;
		TArray<uint32> Indexes;
		meshLod0.IndexBuffer.GetCopy(Indexes);

		if (VertexBuffer)
		{
			if (Indexes.Num() > 0)
			{
				// UE_LOG(LogTemp, Log, TEXT("Triangles for %s - %s"), *GetName(), *Mesh->GetName());
				const int32 VertexCount = VertexBuffer->GetNumVertices();
				for (int32 i = 0; i < Indexes.Num(); i += 3)
				{
					v1pos = VertexBuffer->VertexPosition(Indexes[i]);
					v2pos = VertexBuffer->VertexPosition(Indexes[i + 1]);
					v3pos = VertexBuffer->VertexPosition(Indexes[i + 2]);

					std::array<FVector, 3> vec_array = { v1pos, v2pos, v3pos };

					GatheredGeometry.push_back(vec_array);

					// UE_LOG(LogTemp, Log, TEXT("Tri %d: {%s}, {%s}, {%s}"), i, *vec_array[0].ToString(), *vec_array[1].ToString(), *vec_array[2].ToString());
					//UE_LOG(LogTemp, Log, TEXT("Tri %d: {%s}, {%s}, {%s}"), i, *v1pos.ToString(), *v2pos.ToString(), *v3pos.ToString());
						/*DrawDebugSphere(GetWorld(), v1pos, 10.f, 10, FColor::Green, true);
						DrawDebugSphere(GetWorld(), v2pos, 10.f, 10, FColor::Green, true);
						DrawDebugSphere(GetWorld(), v3pos, 10.f, 10, FColor::Green, true);
						DrawDebugLine(GetWorld(), v1pos, v2pos, FColor::Red, true);
						DrawDebugLine(GetWorld(), v2pos, v3pos, FColor::Green, true);
						DrawDebugLine(GetWorld(), v3pos, v1pos, FColor::Blue, true);*/
				}
			}
		}
	}
}
