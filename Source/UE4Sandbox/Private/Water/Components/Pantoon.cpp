#include "Water/Components/Pantoon.h"
#include "DrawDebugHelpers.h"
#include "Math/UnrealMathUtility.h"
#include "Water/WaveNoise.h"
#include "Water/Components/Mesh/WaterMesh.h"

UPantoon::UPantoon() : Super()
{
	PrimaryComponentTick.bCanEverTick = false;
	bProjectOnPolygons = true;
}

FVector UPantoon::UpdateOffset(IWaterMesh* WaterMesh, const FTransform& ParentTransform)
{
	auto* actorWorld = GetWorld();
	if (!ParentTransform.Equals(FTransform::Identity))
	{
		FTransform outTransform;
		FTransform relTransform = GetRelativeTransform();
		FTransform::Multiply(&outTransform, &relTransform, &ParentTransform);

		CurrentLocation = outTransform.GetLocation();
	}
	else
	{
		CurrentLocation = GetComponentLocation();
	}

	FVector2D loc(CurrentLocation.X, CurrentLocation.Y);

	// Assume that WaterActor centre is in (0, 0, someZ);
	float waterZ = WaterMesh->GetWaterZCoord();
	float zToWater = waterZ - CurrentLocation.Z;
	
	this->LocationOffset = FVector::ZeroVector;

	if (bProjectOnPolygons)
	{
		FVector2D vert1, vert2, vert3;

		if (bDrawDebug)
			DrawDebugPoint(actorWorld, CurrentLocation, 10.f, FColor::Magenta);

		if (WaterMesh->ProjectObjectOnGeometry(loc,
			vert1, vert2, vert3))
		{
			bIsProjectedOnWater = true;

			if (bDrawDebug)
			{
				DrawDebugPoint(actorWorld, FVector(vert1, CurrentLocation.Z), 20.f, FColor::Yellow);
				DrawDebugPoint(actorWorld, FVector(vert2, CurrentLocation.Z), 20.f, FColor::Yellow);
				DrawDebugPoint(actorWorld, FVector(vert3, CurrentLocation.Z), 20.f, FColor::Yellow);
			}

			FVector v1Off = CalcVertexOffset(WaterMesh, vert1, waterZ);
			FVector v2Off = CalcVertexOffset(WaterMesh, vert2, waterZ);
			FVector v3Off = CalcVertexOffset(WaterMesh, vert3, waterZ);

			FVector v1Pos = FVector(vert1, CurrentLocation.Z) + v1Off + (FVector::UpVector * zToWater);
			FVector v2Pos = FVector(vert2, CurrentLocation.Z) + v2Off + (FVector::UpVector * zToWater);
			FVector v3Pos = FVector(vert3, CurrentLocation.Z) + v3Off + (FVector::UpVector * zToWater);

			if (bDrawDebug)
			{
				DrawDebugPoint(actorWorld, v1Pos, 20.f, FColor::Orange);
				DrawDebugPoint(actorWorld, v2Pos, 20.f, FColor::Orange);
				DrawDebugPoint(actorWorld, v3Pos, 20.f, FColor::Orange);

				DrawDebugLine(actorWorld, FVector(vert1, CurrentLocation.Z), FVector(vert2, CurrentLocation.Z), FColor::Yellow);
				DrawDebugLine(actorWorld, FVector(vert2, CurrentLocation.Z), FVector(vert3, CurrentLocation.Z), FColor::Yellow);
				DrawDebugLine(actorWorld, FVector(vert3, CurrentLocation.Z), FVector(vert1, CurrentLocation.Z), FColor::Yellow);

				DrawDebugLine(actorWorld, v1Pos, v2Pos, FColor::Orange);
				DrawDebugLine(actorWorld, v2Pos, v3Pos, FColor::Orange);
				DrawDebugLine(actorWorld, v3Pos, v1Pos, FColor::Orange);
			}

			// TODO: for now it simply projects along Z axis
			// need to do smarter projection if material will move vertexes along X and Y
			TargetLocation = ProjectOnWater(CurrentLocation, CurrentLocation + FVector::DownVector, v1Pos, v2Pos, v3Pos);

			//this->LocationOffset = t1 == t2 && t2 == t3 ?
			//	t1Off :
			//	t1Off * t1i + t2Off * t2i + t3Off * t3i;

			this->LocationOffset = TargetLocation - CurrentLocation;
			//this->LocationOffset.Z -= zToWater;

			if (this->LocationOffset.Z > 0.f)
				bIsOnWater = true;

			FVector surfacePoint = CurrentLocation + this->LocationOffset;
			//FVector surfacePoint = t1Pos * t1i + t2Pos * t2i + t3Pos * t3i;

			FPlane trisPlane(v1Pos, v2Pos, v3Pos);
			CurrentNormal = TargetNormal;
			TargetNormal = trisPlane.GetNormal();
			if (TargetNormal == FVector::ZeroVector) TargetNormal = FVector::UpVector;

			if (bDrawDebug)
			{
				DrawDebugSphere(actorWorld, surfacePoint, 10.f, 10, FColor::Magenta);
				DrawDebugLine(actorWorld, surfacePoint, surfacePoint + TargetNormal * 100.f, FColor::Magenta);
			}
		}
	}
	else
	{
		bIsProjectedOnWater = true;
		
		FVector vOff = CalcVertexOffset(WaterMesh, loc, CurrentLocation.Z + zToWater);
		TargetLocation = FVector(loc, CurrentLocation.Z) + vOff + (FVector::UpVector * zToWater);
		this->LocationOffset = TargetLocation - CurrentLocation;

		if (this->LocationOffset.Z > 0.f)
			bIsOnWater = true;
		
		CurrentNormal = TargetNormal;
		TargetNormal = FVector::UpVector; // TODO: how to calc normal normal?

		if (bDrawDebug)
		{
			DrawDebugPoint(actorWorld, CurrentLocation, 10.f, FColor::Magenta);
			DrawDebugSphere(actorWorld, TargetLocation, 10.f, 10, FColor::Magenta);
			DrawDebugLine(actorWorld, TargetLocation, TargetLocation + TargetNormal * 100.f, FColor::Magenta);
		}
	}

	return this->LocationOffset;
}

FVector UPantoon::ProjectOnWater(FVector L1, FVector L2, FVector T1, FVector T2, FVector T3)
{
	FPlane trisPlane(T1, T2, T3);
	FVector intersection = FMath::LinePlaneIntersection(L1, L2, trisPlane);

	return intersection;
}

FVector UPantoon::CalcVertexOffset(IWaterMesh* WaterMesh, FVector2D p, float Z)
{
	FVector p3D(p.X, p.Y, Z);

	return GetWaveOffset(WaterMesh, p3D);
}

FVector UPantoon::GetWaveOffset(IWaterMesh* WaterMesh, FVector P)
{
	float waveScale = 85.f;
	float waveBaseScale = .5f;
	float waveTile = 206.f;
	float waveSpeed = 50.f;

	WaterMesh->GetMaterialParameters(waveScale, waveBaseScale, waveTile, waveSpeed);

	FVector movingLoc = FVector(1, 0, 1) * GetWorld()->GetTimeSeconds() * waveSpeed;

	auto staticOffset = WaveNoise::ShaderNoise(P / waveTile) * waveBaseScale;
	auto movingOffset = WaveNoise::ShaderNoise((P + movingLoc) / waveTile);

	float zOffset = staticOffset + movingOffset;

	FVector result(0, 0, zOffset);

	result *= waveScale;

	return result;
}
