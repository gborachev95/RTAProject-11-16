#pragma once
#include "Transform.h"
#include <vector>

class KeyFrame
{
public:
	unsigned int m_currFrameNum;
	float m_time;
	std::vector<Transform> m_transforms;
};
