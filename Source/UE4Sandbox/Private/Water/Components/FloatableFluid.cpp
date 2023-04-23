#include "Water/Components/FloatableFluid.h"
#include "Water/Components/Buoyancy.h"
#include "Water/Components/WaterStream.h"
#include "Water/Components/Mesh/WaterMesh.h"

UFloatableFluid::UFloatableFluid() : Super()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UFloatableFluid::BeginPlay()
{
	Super::BeginPlay();

	GetOwner()->GetComponents<UWaterStream>(Streams);
	
	Meshes.Empty();
	auto meshComps = GetOwner()->GetComponentsByInterface(UWaterMesh::StaticClass());
	for (auto* meshComp : meshComps)
	{
		Meshes.Add(meshComp);
	}
}

void UFloatableFluid::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	for (int bi = Buoyants.Num() - 1; bi >= 0; --bi) // its better to iterate in reverse because loop body can change array content
	{
		auto* buoyant = Buoyants[bi];
		
		buoyant->ResetState();
		
		for (auto& waterMesh : Meshes)
			if (waterMesh && waterMesh->GetBoundingBox().Intersect(GetOwner()->GetComponentsBoundingBox(true)))
				buoyant->AccumulateSurfaceMotion((IWaterMesh*)waterMesh.GetInterface());

		for (auto* stream : Streams)
			buoyant->AccumulateStreamMotion(stream);
			
		buoyant->ApplyWaterMotion();
	}
}

void UFloatableFluid::AddBuoyant(UBuoyancy* Buoyant)
{
	Buoyants.AddUnique(Buoyant);
}

void UFloatableFluid::RemoveBuoyant(UBuoyancy* Buoyant)
{
	Buoyants.Remove(Buoyant);
}
