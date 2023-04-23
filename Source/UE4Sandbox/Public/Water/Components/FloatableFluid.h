#pragma once

#include "Components/ActorComponent.h"
#include "Water/WaterStructs.h"
#include "FloatableFluid.generated.h"

class AActor;
class UBuoyancy;
class IWaterMesh;
class UWaterStream;

UCLASS( ClassGroup=(Water), meta=(BlueprintSpawnableComponent) )
class TINYISLANDUE_API UFloatableFluid : public UActorComponent
{
	GENERATED_BODY()

public:	
	UFloatableFluid();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly)
		TArray<UBuoyancy*> Buoyants;

	UPROPERTY()
    TArray<TScriptInterface<IWaterMesh>> Meshes;
	UPROPERTY()
	TArray<UWaterStream*> Streams;

	UFUNCTION()
		void RemoveBuoyant(UBuoyancy* Buoyant);

	UFUNCTION()
		void AddBuoyant(UBuoyancy* Buoyant);
};
