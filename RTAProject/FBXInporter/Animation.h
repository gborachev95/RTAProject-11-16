#pragma once

#include "KeyFrame.h"
#include <vector>

class Animation
{
	unsigned int  m_numKeyFrames;
	float m_totalTime;
	bool m_loop;
public:
	std::vector<KeyFrame> m_keyFrame;

	// Setters
	void SetTotalTime(float _totalTime)
	{
		m_totalTime = _totalTime;
	}

	void SetKeyFramesNumber(unsigned int _numKeyFrames)
	{
		m_numKeyFrames = _numKeyFrames;
	}

	// Getters
	float GetTotalTime()
	{
		return m_totalTime;
	}

	unsigned int GetKeyFramesNumber()
	{
		return m_numKeyFrames;
	}
	
	bool IsLooping()
	{
		return m_loop;
	}
	bool SetLooping(bool _inLoop)
	{
		m_loop = _inLoop;
	}
};

