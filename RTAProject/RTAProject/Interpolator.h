#pragma once

#include "Transform.h"
#include "Animation.h"

class Interpolator
{

	unsigned int previousFrame;
	unsigned int nextFrame;
	float frameTime;
	Animation *animationPtr;
	KeyFrame betweenKeyFrame;  // The result of the interpolation process

	void Process(float _timeToAdd);
	KeyFrame Interpolate(unsigned int previousFrame, unsigned int nextFrame, float delta);

public:
	Interpolator();
	~Interpolator();

};

