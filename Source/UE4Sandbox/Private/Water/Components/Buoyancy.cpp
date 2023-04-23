#include "Water/Components/Buoyancy.h"
#include "Water/Components/Pantoon.h"
#include "Water/Components/FloatableFluid.h"
#include "Core/Components/HoldableComponent.h"
#include "Curves/CurveFloat.h"
#include "DrawDebugHelpers.h"
#include "Water/Components/WaterStream.h"

#pragma region FFloaterConfig
TArray<UPantoon*> FFloaterConfig::FindPantoons()
{
	TArray<UPantoon*> pantoons;

	TArray<USceneComponent*> floaterChildren = Floater->GetAttachChildren();
	for (auto* floaterChild : floaterChildren)
	{
		if (floaterChild->IsA<UPantoon>())
		{
			pantoons.Add(Cast<UPantoon>(floaterChild));
		}
	}

	return pantoons;
}

UPantoon* FFloaterConfig::AddPantoon(AActor* OwningActor, int idx)
{
	FName ObjectName("Pantoon" + FString::FromInt(idx));

	auto* newComp = NewObject<UPantoon>(OwningActor, UPantoon::StaticClass(), ObjectName);
	if (!newComp)
	{
		return NULL;
	}

	newComp->RegisterComponent();
	newComp->bProjectOnPolygons = this->bProjectOnPolygons;
	newComp->bDrawDebug = bDrawDebug;

	return newComp;
}

void FFloaterConfig::Init(AActor* OwningActor)
{
	// collect ref to floater
	auto* floater = CompRef.GetComponent(OwningActor);
	auto* floaterCasted = Cast<USceneComponent>(floater);

	check(floaterCasted != NULL);

	Floater = floaterCasted;
	TransformOriginal = Floater->GetRelativeTransform();

	if (Floater == Floater->GetOwner()->GetRootComponent())
		MoveMode = EMoveMode::RealActor;

	// collect or create pantoons
	TArray<UPantoon*> pantoons;

	// for single
	UPantoon* singlePantoon;
	FVector singleLoc;
	FQuat singleRot;
	UPrimitiveComponent* floaterPrimitive;

	// for corners
	FVector floaterExtents;

	// for custom
	
	auto floaterTr = Floater->GetComponentTransform();
	//auto floaterLoc = floaterTr.GetLocation();
	//auto floaterRot = floaterTr.GetRotation();
	switch (BuoyancyMode)
	{
	case EBuoyancyMode::FromBody:
		// add single pantoon in the center of mass of the floater
		pantoons = FindPantoons();
		if (pantoons.Num() > 0)
		{
			for (int i = pantoons.Num() - 1; i >= 1; i--)
			{
				pantoons[i]->DestroyComponent();
				pantoons.RemoveAt(i);
			}
			singlePantoon = pantoons[0];

			singlePantoon->bDrawDebug = bDrawDebug;
		}
		else
		{
			// create pantoon;
			singlePantoon = AddPantoon(OwningActor);
			singlePantoon->AttachToComponent(Floater, FAttachmentTransformRules::SnapToTargetIncludingScale);
		}

		floaterPrimitive = Cast<UPrimitiveComponent>(floaterCasted);
		singleLoc = floaterPrimitive != NULL ? floaterTr.InverseTransformPosition(floaterPrimitive->GetCenterOfMass()) : FVector::ZeroVector;
		singleRot = FQuat::Identity;
		singlePantoon->SetRelativeLocationAndRotation(singleLoc, singleRot);

		Pantoons.Empty();
		Pantoons.Add(singlePantoon);
		break;
	case EBuoyancyMode::CornerPantoons:
		// add four pantoons at the corners of the boubnding box
		pantoons = FindPantoons();
		if (pantoons.Num() > 0)
		{
			for (int i = pantoons.Num() - 1; i >= 1; i--)
			{
				if (i > 3)
				{
					pantoons[i]->DestroyComponent();
					pantoons.RemoveAt(i);
				}
				else
				{
					pantoons[i]->bDrawDebug = bDrawDebug;
				}
			}
		}

		while (pantoons.Num() < 4)
		{
			auto* newPantoon = AddPantoon(OwningActor, pantoons.Num());
			newPantoon->AttachToComponent(Floater, FAttachmentTransformRules::SnapToTargetIncludingScale);
			pantoons.Add(newPantoon);
		}

		Pantoons = pantoons;

		//floaterPrimitive = Cast<UPrimitiveComponent>(floaterCasted);
		//auto bounds = Floater->CalcLocalBounds();
		floaterExtents = Floater->Bounds.BoxExtent;

		// UE_LOG(LogTemp, Warning, TEXT("Floater extents (%f, %f, %f)"), floaterExtents.X, floaterExtents.Y, floaterExtents.Z);
		Pantoons[0]->SetRelativeLocationAndRotation(FVector(floaterExtents.X, floaterExtents.Y, 0.f), FQuat::Identity);
		Pantoons[1]->SetRelativeLocationAndRotation(FVector(floaterExtents.X, -floaterExtents.Y, 0.f), FQuat::Identity);
		Pantoons[2]->SetRelativeLocationAndRotation(FVector(-floaterExtents.X, -floaterExtents.Y, 0.f), FQuat::Identity);
		Pantoons[3]->SetRelativeLocationAndRotation(FVector(-floaterExtents.X, floaterExtents.Y, 0.f), FQuat::Identity);
		
		break;
	case EBuoyancyMode::CustomPantoons:
		Pantoons = FindPantoons();
		break;
	}
}

void FFloaterConfig::Clear()
{
	// clear ref to floater
	Floater = NULL;

	// destroy and clear pantoons
	if (BuoyancyMode != EBuoyancyMode::CustomPantoons)
	{
		Pantoons.Reset();
	}
}

FTransform FFloaterConfig::GetOriginalTransform() const
{
	FTransform outTransform;
	auto* parent = Floater->GetAttachParent();
	FTransform parentTransform = parent != NULL ? parent->GetComponentTransform() : Floater->GetComponentTransform();
	FTransform::Multiply(&outTransform, &TransformOriginal, &parentTransform);
	return outTransform;
}

void FFloaterConfig::ResetTransform() const
{
	if (MoveMode != EMoveMode::RealActor)
		Floater->SetRelativeTransform(TransformOriginal);
}

void FFloaterConfig::AddTickMovement(FVector MoveDelta, FQuat RotateDelta)
{
	TickMoveDelta += MoveDelta;
	TickRotateDelta = RotateDelta * TickRotateDelta;
}

void FFloaterConfig::ApplyTickMovement()
{
	if (IsPhysical())
	{
		auto* primitiveFloater = Cast<UPrimitiveComponent>(Floater);
		primitiveFloater->AddForce(TickMoveDelta, NAME_None, true);
		primitiveFloater->AddTorqueInRadians(TickRotateDelta.GetRotationAxis() * TickRotateDelta.GetAngle(), NAME_None, true);
	}
	else
	{
		if (MoveMode == EMoveMode::Primitive)
		{
			Floater->AddWorldOffset(TickMoveDelta);
			Floater->AddWorldRotation(TickRotateDelta);
		}
		else if (MoveMode == EMoveMode::RealActor)
		{
			Floater->GetOwner()->AddActorWorldOffset(TickMoveDelta);
			Floater->GetOwner()->AddActorWorldRotation(TickRotateDelta);
		}
	}

	TickMoveDelta = FVector::ZeroVector;
	TickRotateDelta = FQuat::Identity;
}
#pragma endregion // FFloaterConfig

#pragma region UBuoyancy
UBuoyancy::UBuoyancy() : Super()
{
	PrimaryComponentTick.bCanEverTick = false;

	ConstructorHelpers::FObjectFinder<UCurveFloat> curveFinder(TEXT("/Game/BP/Water/AnimatedFloating"));
	if (curveFinder.Succeeded())
	{
		FloatingCurve = curveFinder.Object;
	}
}

void UBuoyancy::BeginPlay()
{
	Super::BeginPlay();

	auto* ownerActor = GetOwner();

	// collect floaters
	// and save floater origin
	FloaterConfig.Init(ownerActor);

	// bind to actor ovelap events
	ownerActor->OnActorBeginOverlap.AddDynamic(this, &UBuoyancy::OnOwnerOverlapBegin);
	ownerActor->OnActorEndOverlap.AddDynamic(this, &UBuoyancy::OnOwnerOverlapEnd);

    GetWorld()->GetTimerManager().SetTimerForNextTick([&](){ // Add short delay because order of BeginPlay and UpdateOverlaps isn't reliable
		if (HoldableComp != nullptr)
		{
			if (HoldableComp->InHold)
				OnOwnerHeld(HoldableComp);
			else
				OnOwnerReleased(HoldableComp);
		}

		// check initial overlaps
		TArray<AActor*> initialOverlapers;
		GetOwner()->GetOverlappingActors(initialOverlapers);
		for (auto* overlapper : initialOverlapers)
		{
			OnOwnerOverlapBegin(ownerActor, overlapper);
		}
	});
}

void UBuoyancy::TouchWater(UFloatableFluid* Water)
{
	if (!bCanFloat || (HoldableComp && HoldableComp->InHold))
		return;

	// snap to surface level
	//FVector loc = ownerActor->GetActorLocation();
	//loc.Z = GetOwner()->GetActorLocation().Z;
	//ownerActor->SetActorLocation(loc);

	//// rotate normal to up
	//FVector buoyantForward = ownerActor->GetActorForwardVector();
	//FRotator rot = FRotationMatrix::MakeFromZX(FVector::UpVector, buoyantForward).Rotator();
	//ownerActor->SetActorRotation(rot);

	Water->AddBuoyant(this);
	TouchingWaters.Add(Water);

	OnAttachToWater.Broadcast(Water, this);
}

void UBuoyancy::LeaveWater(UFloatableFluid* Water)
{
	if (!FloaterConfig.IsPhysical())
	{
		FloaterConfig.ResetTransform();
	}

	// LeaveWaterInternal(Water);
	Water->RemoveBuoyant(this);
	TouchingWaters.Remove(Water);

	FloatingStage = EFloatingStage::NotFloating;

	OnDetachFromWater.Broadcast(Water, this);
}

void UBuoyancy::ResetState()
{
	int floatingPantoons = 0;
	for (auto* floaterPantoon : FloaterConfig.Pantoons)
	// for (int i = 0; i < pantoonsNum; i++)
	{
		if (floaterPantoon->bIsOnWater)
			++floatingPantoons;

		floaterPantoon->bIsOnWater = false;
		floaterPantoon->bIsProjectedOnWater = false;
	}
	
	if (floatingPantoons == 0)
		FloatingStage = EFloatingStage::TouchingWater;
	else if (floatingPantoons == FloaterConfig.Pantoons.Num())
		FloatingStage = EFloatingStage::FullFloating;
	else
		FloatingStage = EFloatingStage::PartiallyFloating;
}

void UBuoyancy::AccumulateSurfaceMotion(IWaterMesh* WaterMesh)
{
	if (!FloaterConfig.IsValid())
		return;

	auto* actorWorld = GetOwner()->GetWorld();
	
	bool physicsMovement = FloaterConfig.IsPhysical();

	auto* floater = FloaterConfig.Floater;
	auto pantoons = FloaterConfig.Pantoons;
	// auto floaterTr = physicsMovement ? floater->GetComponentTransform() : FloaterConfig.GetOriginalTransform();
	auto floaterTr = floater->GetComponentTransform();
	auto floaterLoc = floaterTr.GetLocation();
	auto floaterRot = floaterTr.GetRotation();

	TArray<FVector> pantoonLocs; // WS
	TArray<FVector> newPantoonLocs; // WS
	TArray<FVector> pantoonOffsets; // WS
	int32 pantoonsNum = pantoons.Num();

	if (pantoonsNum == 0)
		return;

	int32 affectedPantoons = 0;
	for (int32 i = 0; i < pantoonsNum; i++)
	{
		auto* pantoon = pantoons[i];

		pantoonLocs.Add(pantoon->GetComponentLocation());
		FVector pantoonOffset = pantoon->UpdateOffset(WaterMesh);
		pantoonOffsets.Add(pantoonOffset);
		newPantoonLocs.Add(pantoonLocs[i] + pantoonOffsets[i]);

		if (pantoonOffset != FVector::ZeroVector)
			++affectedPantoons;
	}

	if (affectedPantoons == 0)
		return;

	// calc origin
	FVector locSumm(0.f);
	for (auto pantoonLoc : pantoonLocs)
	{
		locSumm += pantoonLoc;
	}
	FVector originLoc = locSumm / pantoonsNum; // WS

	// calc cur origin pos
	FVector fullOffset(0.f);
	for (const auto& offset : pantoonOffsets)
	{
		fullOffset += offset;
	}
	fullOffset /= pantoonsNum;
	
	FVector originOffset = fullOffset; // WS
	originOffset.Z = FMath::Max(0.f, originOffset.Z);
	auto newOriginLoc = originLoc + originOffset; // WS

	// calc cur origin normal
	// normal as middle of normals of the trises from center with each edge
	FVector originNormal = CalcPantoonsNormal(originLoc, pantoons);
	FVector newOriginNormal = CalcPantoonsNormal(originLoc + fullOffset, pantoons, true);

	auto normalOffset = FQuat::FindBetween(originNormal, newOriginNormal); // WS

	if (bDrawDebug)
	{
		DrawDebugPoint(actorWorld, originLoc, 10.f, FColor::Yellow);
		DrawDebugLine(actorWorld,
			originLoc,
			originLoc + originNormal * 50.f,
			FColor::Yellow);

		DrawDebugSphere(actorWorld, newOriginLoc, 10.f, 10, FColor::White);
		DrawDebugLine(actorWorld,
			newOriginLoc,
			newOriginLoc + newOriginNormal * 50.f,
			FColor::White);
	}

	// move by waves
	auto newLoc = floaterLoc + originOffset;
	auto newRot = normalOffset * floaterRot;

	if (physicsMovement)
	{
		auto* primitiveFloater = Cast<UPrimitiveComponent>(floater);
		auto forceVec = originOffset;

		if (forceVec.Z > 0.f)
		{
			auto torqueVec = normalOffset;

			forceVec /= 5.f;
			forceVec *= forceVec * forceVec;
			forceVec = forceVec.GetClampedToMaxSize(250.f); // pure force
			if (FloatingStage == EFloatingStage::FullFloating)
				forceVec += FVector::UpVector * -actorWorld->GetGravityZ() * 1.1f; // add anti gravity
			FloaterConfig.AddTickMovement(forceVec, torqueVec);
		}
		else
		{
			// FloaterConfig.AddTickMovement(FVector::UpVector * -actorWorld->GetGravityZ() * 0.7f);
		}
	}
	else
	{
		float floatingOffset {0};
		auto* primitiveFloater = Cast<UPrimitiveComponent>(floater);

		if (FloatingCurve != NULL)
		{
			if (FloatingStage == EFloatingStage::FullFloating || FloatingStage == EFloatingStage::PartiallyFloating)
			{
				floatingOffset = FloatingCurve->GetFloatValue(actorWorld->GetTimeSeconds());
			}
			else if (FloatingStage == EFloatingStage::TouchingWater)
			{
				floatingOffset = 20.f + actorWorld->GetGravityZ() * actorWorld->GetDeltaSeconds();
			}
		}

		FloaterConfig.AddTickMovement(FVector(0, 0, originOffset.Z - 20.f + floatingOffset));
	}
}

void UBuoyancy::AccumulateStreamMotion(UWaterStream* WaterStream)
{
	if (!FloaterConfig.IsValid())
		return;
	
	auto* actorWorld = GetOwner()->GetWorld();

	bool physicsMovement = FloaterConfig.IsPhysical();
	auto* primitiveFloater = Cast<UPrimitiveComponent>(FloaterConfig.Floater);

	// auto floaterTr = physicsMovement ? FloaterConfig.Floater->GetComponentTransform() : FloaterConfig.GetOriginalTransform();
	auto floaterTr = FloaterConfig.Floater->GetComponentTransform();
	auto floaterLoc = floaterTr.GetLocation();
	auto floaterRot = floaterTr.GetRotation();

	// floaterLoc = physicsMovement ? newLoc : newOriginLoc;

	auto streamInputKey = WaterStream->FindInputKeyClosestToWorldLocation(floaterLoc);

	auto locationSpline = WaterStream->GetLocationAtSplineInputKey(streamInputKey, ESplineCoordinateSpace::World);

	auto distanceToSpline = (floaterLoc - locationSpline).Size();

	if (bDrawDebug)
	{
		DrawDebugLine(actorWorld, locationSpline, floaterLoc,
			distanceToSpline > WaterStream->InfluenceDistance ? FColor::Red : FColor::Green);
	}

	if (distanceToSpline > WaterStream->InfluenceDistance)
		return;

	auto distanceKoef = FMath::Clamp(1.f / distanceToSpline, 0.3f, 1.1f);
	FVector directionSpline = WaterStream->GetDirectionAtSplineInputKey(streamInputKey, ESplineCoordinateSpace::World);
	// FVector tangentSpline = WaterStream->GetTangentAtSplineInputKey(streamInputKey, ESplineCoordinateSpace::World);

	if (bDrawDebug)
	{
		DrawDebugSphere(actorWorld, floaterLoc + directionSpline * 0.01f, 50.f, 10, FColor::Orange);
	}

	if (physicsMovement)
	{
		auto curVelocity = primitiveFloater->GetPhysicsLinearVelocity();
		auto curVelocity2D = FVector(curVelocity.X, curVelocity.Y, 0.f);
		auto velChange = curVelocity2D.ProjectOnTo(directionSpline.RotateAngleAxis(90.f, FVector::UpVector)) * 10.f;
		FloaterConfig.AddTickMovement(directionSpline * 100.f * WaterStream->Force * distanceKoef - velChange);
	}
	else
	{
		auto moveVector = floaterLoc + directionSpline * WaterStream->Force * distanceKoef;
		float realOffsetX = moveVector.X - floaterLoc.X;
		float realOffsetY = moveVector.Y - floaterLoc.Y;
		FloaterConfig.AddTickMovement(FVector(realOffsetX, realOffsetY, 0));
	}
}

void UBuoyancy::ApplyWaterMotion()
{
	if (!FloaterConfig.IsValid())
		return;

	FloaterConfig.ApplyTickMovement();
}

void UBuoyancy::OnOwnerOverlapBegin(AActor* OverlappedActor, AActor* OtherActor)
{
	if (auto* otherWater = OtherActor->FindComponentByClass<UFloatableFluid>())
	if (!TouchingWaters.Contains(otherWater) &&
		(HoldableComp == NULL || !HoldableComp->InHold))
		TouchWater(otherWater);
}

void UBuoyancy::OnOwnerOverlapEnd(AActor* OverlappedActor, AActor* OtherActor)
{
	if (auto* otherWater = OtherActor->FindComponentByClass<UFloatableFluid>())
	if (TouchingWaters.Contains(otherWater) &&
		(HoldableComp == NULL || !HoldableComp->InHold))
		LeaveWater(otherWater);
}

FVector UBuoyancy::CalcPantoonsNormal(FVector OriginLoc, TArray<UPantoon*> Pantoons, bool bTarget)
{
	int pantoonsNum = Pantoons.Num();
	
	check(pantoonsNum > 0);
	
	FVector resultNormal;
	if (pantoonsNum > 2)
	{
		FVector normalSumm(0.f);
		for (int i = 0; i < pantoonsNum; i++)
		{
			FVector t1 = bTarget ? Pantoons[i]->TargetLocation : Pantoons[i]->CurrentLocation;
			int nextIdx = i == pantoonsNum-1 ? 0 : i + 1;
			FVector t2 = bTarget ? Pantoons[nextIdx]->TargetLocation : Pantoons[nextIdx]->CurrentLocation;
			FVector t3 = OriginLoc;

			FPlane trisPlane(t1, t2, t3);
			FVector trisNormal = trisPlane.GetNormal(); // WS

			if (bTarget && FVector::DotProduct(FVector::UpVector, trisNormal) < 0.f ||
				FVector::DotProduct(GetOwner()->GetActorUpVector(), trisNormal) < 0.f)
				trisNormal *= -1.f;
			// UE_LOG(LogTemp, Warning, TEXT("tris normal (%f)"), trisNormal.X);
			// here trisNormal sometimes always undefined vector
			// it leads to resultNormal = (0, 90, 0)
			normalSumm += trisNormal;

			//FVector trisCenter = (t1 + t2 + t3) / 3.f;
			//auto* actorWorld = GetWorld();
			//DrawDebugLine(actorWorld,
			//	trisCenter,
			//	trisCenter + trisNormal * 100.f,
			//	FColor::Red);
			//DrawDebugLine(actorWorld, t1, t2, FColor::Cyan);
			//DrawDebugLine(actorWorld, t2, t3, FColor::Cyan);
			//DrawDebugLine(actorWorld, t1, t3, FColor::Cyan);
		}
		resultNormal = normalSumm / pantoonsNum;
	}
	else if (pantoonsNum > 1)
	{
		FVector normalSumm(0.f);
		normalSumm += bTarget ? Pantoons[0]->TargetNormal : Pantoons[0]->CurrentNormal;
		normalSumm += bTarget ? Pantoons[1]->TargetNormal : Pantoons[1]->CurrentNormal;

		resultNormal = normalSumm / pantoonsNum;
	}
	else
	{
		resultNormal = bTarget ? Pantoons[0]->TargetNormal : GetOwner()->GetActorUpVector();
	}

	return resultNormal.GetSafeNormal();
}
#pragma endregion // UBuoyancy
