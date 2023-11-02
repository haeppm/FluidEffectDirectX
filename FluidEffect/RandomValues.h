#pragma once
#include <random>

class RandomValues
{
public:
	static float GetRandomValue(float min, float max) {
		static std::random_device rd;
		static std::mt19937 gen(rd());
		static std::uniform_real_distribution<float> dis(min, max);
		return dis(gen);
	}
};

