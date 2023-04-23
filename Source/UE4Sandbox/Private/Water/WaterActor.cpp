#include "Water/WaterActor.h"
#include "Components/BoxComponent.h"
#include "NavModifierComponent.h"
// #include "Water/Components/FloatableFluid.h"
#include "Water/Components/WaterStream.h"
#include "Water/Components/Mesh/CustomWaterMesh.h"
#include "Water/Components/Mesh/CircleSegmentWaterMesh.h"
#include "Water/Components/Mesh/SquareWaterMesh.h"

AWaterActor::AWaterActor() : Super()
{
	RootComponent = CreateDefaultSubobject<USceneComponent>("Root");

	OverlapComp = CreateDefaultSubobject<UBoxComponent>("BoxCollision");
	OverlapComp->SetupAttachment(RootComponent);
	OverlapComp->SetCollisionProfileName("Trigger");
	OverlapComp->SetGenerateOverlapEvents(true);

	FluidComp = CreateDefaultSubobject<UFloatableFluid>("FloatableFluid");

	NavModifierComp = CreateDefaultSubobject<UNavModifierComponent>("NavModifier");
	NavModifierComp->AreaClass = UNavArea_Water::StaticClass();
}

#if WITH_EDITOR
void AWaterActor::AddStream()
{
	if (auto* newStreamComp = NewObject<UWaterStream>(this))
	{
		AddInstanceComponent(newStreamComp);
		newStreamComp->SetupAttachment(RootComponent);
		newStreamComp->RegisterComponent();

		FSelectionStateOfLevel selectionState;
		GEditor->SetSelectionStateOfLevel(selectionState);
		selectionState.SelectedActors.Add(GetPathName());
		GEditor->SetSelectionStateOfLevel(selectionState);
	}
}

void AWaterActor::AddCustomMesh()
{
	if (auto* newMeshComp = NewObject<UCustomWaterMesh>(this))
	{
		AddInstanceComponent(newMeshComp);
		newMeshComp->SetupAttachment(RootComponent);
		newMeshComp->RegisterComponent();

		FSelectionStateOfLevel selectionState;
		GEditor->SetSelectionStateOfLevel(selectionState);
		selectionState.SelectedActors.Add(GetPathName());
		GEditor->SetSelectionStateOfLevel(selectionState);
	}
}

void AWaterActor::AddCircularMesh()
{
	if (auto* newMeshComp = NewObject<UCircleSegmentWaterMesh>(this))
	{
		AddInstanceComponent(newMeshComp);
		newMeshComp->SetupAttachment(RootComponent);
		newMeshComp->RegisterComponent();

		FSelectionStateOfLevel selectionState;
		GEditor->SetSelectionStateOfLevel(selectionState);
		selectionState.SelectedActors.Add(GetPathName());
		GEditor->SetSelectionStateOfLevel(selectionState);
	}
}

void AWaterActor::AddSquaresMesh()
{
	if (auto* newMeshComp = NewObject<USquareWaterMesh>(this))
	{
		AddInstanceComponent(newMeshComp);
		newMeshComp->SetupAttachment(RootComponent);
		newMeshComp->RegisterComponent();

		FSelectionStateOfLevel selectionState;
		GEditor->SetSelectionStateOfLevel(selectionState);
		selectionState.SelectedActors.Add(GetPathName());
		GEditor->SetSelectionStateOfLevel(selectionState);
	}
}
#endif // WITH_EDITOR
