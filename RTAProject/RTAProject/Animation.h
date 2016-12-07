#pragma once

#include "KeyFrame.h"
#include <vector>

class Animation
{
public:
	unsigned int  m_num_KeyFrames;
	float m_totalTime;
	std::vector<KeyFrame> m_bones;
	const char* m_name;
};

