#pragma once

/*#include "Transform.h"
#include "Animation.h"*/
#include "includes.h"
#include <vector>

class Interpolator
{

	unsigned int previousFrame;
	unsigned int nextFrame;
	float frameTime;
	Animation *animationPtr;
	KeyFrame betweenKeyFrame;  // The result of the interpolation process

public:
	Interpolator();
	~Interpolator();

	std::vector<Transform> Interpolate(std::vector<Transform> previousFrame, std::vector<Transform> nextFrame, float prevFrameTime, float nextFrameTime, float currTime);
};

