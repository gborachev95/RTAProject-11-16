#pragma once

#include "KeyFrame.h"

class Animation
{

	int num_KeyFrames;
	float totalTime;
	KeyFrame * bones;

public:
	Animation();
	~Animation();
};

