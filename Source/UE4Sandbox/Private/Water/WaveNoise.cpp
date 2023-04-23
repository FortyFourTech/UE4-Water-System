#include "Water/WaveNoise.h"

 float WaveNoise::ShaderNoise(FVector V)
{
	float seed000, seed001, seed010, seed011, seed100, seed101, seed110, seed111;
	FVector fv = NoiseSeeds(V, seed000, seed001, seed010, seed011, seed100, seed101, seed110, seed111);

	float rand000 = RandBBSfloat(seed000) * 2.f - 1.f;
	float rand100 = RandBBSfloat(seed100) * 2.f - 1.f;
	float rand010 = RandBBSfloat(seed010) * 2.f - 1.f;
	float rand110 = RandBBSfloat(seed110) * 2.f - 1.f;
	float rand001 = RandBBSfloat(seed001) * 2.f - 1.f;
	float rand101 = RandBBSfloat(seed101) * 2.f - 1.f;
	float rand011 = RandBBSfloat(seed011) * 2.f - 1.f;
	float rand111 = RandBBSfloat(seed111) * 2.f - 1.f;

	FVector Weights = PerlinRamp(fv);

	float i = FMath::Lerp(FMath::Lerp(rand000, rand100, Weights.X), FMath::Lerp(rand010, rand110, Weights.X), Weights.Y);
	float j = FMath::Lerp(FMath::Lerp(rand001, rand101, Weights.X), FMath::Lerp(rand011, rand111, Weights.X), Weights.Y);
	return FMath::Lerp(i, j, Weights.Z);
}

FVector WaveNoise::NoiseSeeds(FVector V,
	float& Seed000, float& Seed001, float& Seed010, float& Seed011,
	float& Seed100, float& Seed101, float& Seed110, float& Seed111)
{
	FVector fv(FMath::Frac(V.X), FMath::Frac(V.Y), FMath::Frac(V.Z));
	FVector iv(FMath::Floor(V.X), FMath::Floor(V.Y), FMath::Floor(V.Z));

	FVector primes(19.f, 47.f, 101.f);

	Seed000 = FVector::DotProduct(iv, primes);
	Seed100 = Seed000 + primes.X;
	Seed010 = Seed000 + primes.Y;
	Seed110 = Seed100 + primes.Y;
	Seed001 = Seed000 + primes.Z;
	Seed101 = Seed100 + primes.Z;
	Seed011 = Seed010 + primes.Z;
	Seed111 = Seed110 + primes.Z;

	return fv;
}

float WaveNoise::RandBBSfloat(float Seed)
{
	float BBS_PRIME24 = 4093.f;
	float s = FMath::Frac(Seed / BBS_PRIME24);
	s = FMath::Frac(s * s * BBS_PRIME24);
	s = FMath::Frac(s * s * BBS_PRIME24);
	return s;
}

FVector WaveNoise::PerlinRamp(FVector T)
{
	return T * T * T * (T * (T * 6.f - FVector(15.f, 15.f, 15.f)) + FVector(10.f, 10.f, 10.f));
}
