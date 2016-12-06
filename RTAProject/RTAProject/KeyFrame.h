#pragma once
#include "Transform.h"

class KeyFrame
{
	int num_bones;
	float time;
	Transform* pos;

public:
	KeyFrame();
	~KeyFrame();
};

