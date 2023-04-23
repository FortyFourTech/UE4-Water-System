#pragma once

#include "Components/ActorComponent.h"
#include "Buoyancy.generated.h"

class UPantoon;
class UFloatableFluid;
class UHoldableComponent;
class UCurveFloat;
class IWaterMesh;
class UWaterStream;

UENUM(BlueprintType)
enum class EMoveMode : uint8
{
	Primitive = 0,
	RealActor = 1,
};

UENUM(BlueprintType)
enum class EBuoyancyMode : uint8
{
	FromBody		= 0, // UMETA(DisplayName = "Without pantoons")
	CornerPantoons	= 1,
	CustomPantoons	= 2,
};

UENUM(BlueprintType)
enum class EFloatingStage : uint8
{
	NotFloating			= 0, // UMETA(DisplayName = "Without pantoons")
	TouchingWater		= 1,
	PartiallyFloating	= 2,
	FullFloating		= 3,
};

USTRUCT(BlueprintType)
struct FFloaterConfig
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, meta=(UseComponentPicker, AllowedClasses="PrimitiveComponent"))
		FComponentReference CompRef;

	UPROPERTY(EditAnywhere)
		EBuoyancyMode BuoyancyMode;

	UPROPERTY(EditAnywhere)
		EMoveMode MoveMode;

	UPROPERTY(VisibleAnywhere) 
		USceneComponent* Floater;

	UPROPERTY(VisibleAnywhere)
		TArray<UPantoon*> Pantoons;

	UPROPERTY(EditAnywhere)
		bool bProjectOnPolygons; // false saves computations

	UPROPERTY(EditAnywhere)
		bool bDrawDebug;

private:
	FVector TickMoveDelta;
	FQuat TickRotateDelta;

public:
	FFloaterConfig()
	{
		bProjectOnPolygons = true;
	}

	bool IsValid() const { return Floater != NULL; }

	bool IsPhysical() const
	{
		auto* primitiveFloater = Cast<UPrimitiveComponent>(Floater);
		return primitiveFloater != NULL && primitiveFloater->IsSimulatingPhysics();
	}

	void Init(AActor* OwningActor);

	void Clear();

	FTransform GetOriginalTransform() const;

	void ResetTransform() const;

	void AddTickMovement(FVector MoveDelta, FQuat RotateDelta = FQuat::Identity);
	void ApplyTickMovement();

private:
	FTransform TransformOriginal;

	TArray<UPantoon*> FindPantoons();

	UPantoon* AddPantoon(AActor* OwningActor, int idx = 0);
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FBuoyantDelegate, UFloatableFluid*, Water, UBuoyancy*, Buoyant);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FBuoyantDetachDelegate);

UCLASS( ClassGroup=(Water), meta=(BlueprintSpawnableComponent) )
class TINYISLANDUE_API UBuoyancy : public UActorComponent
{
	GENERATED_BODY()

public:
	UBuoyancy();

protected:
	virtual void BeginPlay() override;

// Events
public:
	UPROPERTY(BlueprintAssignable)
		FBuoyantDelegate OnAttachToWater;
	UPROPERTY(BlueprintAssignable)
		FBuoyantDelegate OnDetachFromWater;

// Properties
public:
	UPROPERTY(EditAnywhere)
		FFloaterConfig FloaterConfig;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool bCanFloat = true;

	UPROPERTY(EditAnywhere)
		bool bDrawDebug = false;

protected:
	UPROPERTY()
		UHoldableComponent* HoldableComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		EFloatingStage FloatingStage;

	UPROPERTY()
		UCurveFloat* FloatingCurve;

	UPROPERTY(VisibleInstanceOnly, Category = Water)
	TSet<UFloatableFluid*> TouchingWaters;

// Functions
public:
	UFUNCTION(BlueprintCallable, BlueprintPure)
	virtual bool IsFloating() { return (uint8_t)FloatingStage > 1; }
	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool IsOnWater() { return TouchingWaters.Num() > 0; }

	UFUNCTION(BlueprintCallable)
	virtual void TouchWater(UFloatableFluid* Water);

	UFUNCTION(BlueprintCallable)
	virtual void LeaveWater(UFloatableFluid* Water);

	void ResetState();
	virtual void AccumulateSurfaceMotion(IWaterMesh* WaterMesh);
	virtual void AccumulateStreamMotion(UWaterStream* WaterStream);
	virtual void ApplyWaterMotion();

// Functions
protected:
	UFUNCTION()
	void OnOwnerOverlapBegin(AActor* OverlappedActor, AActor* OtherActor);

	UFUNCTION()
	void OnOwnerOverlapEnd(AActor* OverlappedActor, AActor* OtherActor);

	FVector CalcPantoonsNormal(FVector originLoc, TArray<UPantoon*> pantoons, bool bTarget = false);
};
