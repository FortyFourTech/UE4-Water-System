#pragma once

class WaveNoise
{
public:
	static float ShaderNoise(FVector P);

private:
	static FVector NoiseSeeds(FVector V,
		float& Seed000, float& Seed001, float& Seed010, float& Seed011,
		float& Seed100, float& Seed101, float& Seed110, float& Seed111);

	static float RandBBSfloat(float Seed);

	static FVector PerlinRamp(FVector T);
};
