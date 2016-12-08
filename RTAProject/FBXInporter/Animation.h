#pragma once

#include "KeyFrame.h"
#include <vector>

class Animation
{
	unsigned int  m_numKeyFrames;
	float m_totalTime;
	const char* m_name;
public:
	std::vector<KeyFrame> m_keyFrame;

	// Setters
	void SetAnimationName(const char* _name)
	{
		m_name = _name;
	}

	void SetTotalTime(float _totalTime)
	{
		m_totalTime = _totalTime;
	}

	void SetKeyFramesNumber(unsigned int _numKeyFrames)
	{
		m_numKeyFrames = _numKeyFrames;
	}

	// Getters
	const char* GetAnimationName()
	{
		return m_name;
	}

	float GetTotalTime()
	{
		return m_totalTime;
	}

	unsigned int GetKeyFramesNumber()
	{
		return m_numKeyFrames;
	}
};

