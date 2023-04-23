#pragma once

struct FRadCacheParams
{
public:
	float StartRadius = 10.f;
	float SegmentAngle = 2.f;
	int32 RadsNum = 50;

	FRadCacheParams()
	{}
	
	FRadCacheParams(float StartRadius, float SegmentAngle, int32 RadsNum)
    : StartRadius(StartRadius), SegmentAngle(SegmentAngle), RadsNum(RadsNum)
	{}

	bool operator==(const FRadCacheParams& Other) const
	{
		return Equals(Other);
	}

	bool Equals(const FRadCacheParams& Other) const
	{
		return StartRadius == Other.StartRadius &&
			SegmentAngle == Other.SegmentAngle &&
			RadsNum == Other.RadsNum;
	}
};

#if UE_BUILD_DEBUG
uint32 GetTypeHash(const FRadCacheParams& Thing);
#else // optimize by inlining in shipping and development builds
FORCEINLINE uint32 GetTypeHash(const FRadCacheParams& Thing)
{
	uint32 Hash = FCrc::MemCrc32(&Thing, sizeof(FRadCacheParams));
	return Hash;
}
#endif

class RadialCache
{
private:
	static TMap<FRadCacheParams, TArray<float>> Caches;

public:
	/// @brief 
	/// @param StartRadius should be > 0.f
	/// @param SegmentAngle should be > 0.f
	/// @param RadsNum should be > 1
	/// @return 
	static const TArray<float>& GetRadialCache(float StartRadius, float SegmentAngle, int32 RadsNum);
	
private:
	static TArray<float> GenerateCache(const FRadCacheParams& Params);
};
