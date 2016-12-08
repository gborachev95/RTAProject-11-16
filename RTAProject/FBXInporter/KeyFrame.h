#pragma once
#include "Transform.h"
#include <vector>

class KeyFrame
{
	float m_time;
	unsigned int m_boneIndex;
	
public:
	std::vector<Transform> m_bones;
	// Setters
	void SetFrameTime(float _time)
	{
		m_time = _time;
	}
	void SetBoneIndex(unsigned int _boneIndex)
	{
		m_boneIndex = _boneIndex;
	}

	// Getters
	float GetFrameTime()
	{
		return m_time;
	}
	unsigned int GetBoneIndex()
	{
		return m_boneIndex;
	}
};
