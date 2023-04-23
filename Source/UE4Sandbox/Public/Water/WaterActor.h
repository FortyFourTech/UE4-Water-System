#pragma once

#include "GameFramework/Actor.h"
#include "Water/WaterStructs.h"
#include "WaterActor.generated.h"

class UBoxComponent;
class UFloatableFluid;
class UNavModifierComponent;
class UStaticMeshComponent;
class USceneComponent;

UCLASS(Blueprintable, BlueprintType)
class TINYISLANDUE_API AWaterActor : public AActor
{
	GENERATED_BODY()

public:
	AWaterActor();

// Components
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Water)
	UBoxComponent* OverlapComp;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Water)
	UFloatableFluid* FluidComp;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Water)
	UNavModifierComponent* NavModifierComp;

// Functions
#if WITH_EDITOR
public:
    UFUNCTION(CallInEditor, Category = Water)
    void AddStream();
    UFUNCTION(CallInEditor, Category = Water, Meta = (DisplayName = "Add mesh (custom)"))
    void AddCustomMesh();
    UFUNCTION(CallInEditor, Category = Water, Meta = (DisplayName = "Add mesh (circle segment)"))
    void AddCircularMesh();
    UFUNCTION(CallInEditor, Category = Water, Meta = (DisplayName = "Add mesh (instanced squares)"))
    void AddSquaresMesh();
#endif // WITH_EDITOR
};
