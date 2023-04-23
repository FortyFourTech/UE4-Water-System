#include "Water/WaterStructs.h"

TMap<FRadCacheParams, TArray<float>> RadialCache::Caches = TMap<FRadCacheParams, TArray<float>>();

const TArray<float>& RadialCache::GetRadialCache(float StartRadius, float SegmentAngle, int32 RadsNum)
{
    FRadCacheParams cacheKey (StartRadius, SegmentAngle, RadsNum);
    if (!Caches.Contains(cacheKey))
    {
        auto newCache = GenerateCache(cacheKey);
        Caches.Add(cacheKey, newCache);
    }

    return Caches[cacheKey];
}

TArray<float> RadialCache::GenerateCache(const FRadCacheParams& Params)
{
    TArray<float> array;

    float a2 = FMath::DegreesToRadians(Params.SegmentAngle) / 2.f;
    float r = Params.StartRadius;
    array.Add(r);

    for (int32 i = 1; i < Params.RadsNum; i++)
    {
        array.Add(array[i - 1] + ((2.f * array[i - 1] * FMath::Sin(a2)) / (FMath::Cos(a2) - FMath::Sin(a2))));
    }
    return array;
}
