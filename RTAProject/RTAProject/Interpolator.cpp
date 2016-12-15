#include "Interpolator.h"
#include <DirectXMath.h>

using namespace DirectX;

Interpolator::Interpolator()
{
}

Interpolator::~Interpolator()
{
}

std::vector<Transform> Interpolator::Interpolate(std::vector<Transform> previousFrame, std::vector<Transform> nextFrame, float prevFrameTime, float nextFrameTime, float currTime)
{
	//Interpolating between two key frame using XMMath…

	XMVECTOR scaleCur, rotationCur, positionCur, scaleNext, rotationNext, positionNext;
	std::vector<Transform> ret;
	Transform trns; //Fill in before getting it into ret
	float timeDiff = nextFrameTime - prevFrameTime;
	float timeSincePrevFrame = currTime - prevFrameTime;
	float ratio = timeSincePrevFrame / timeDiff;

	//for each bone
	for (size_t i = 0; i < previousFrame.size(); i++)
	{
		XMMatrixDecompose(&scaleCur, &rotationCur, &positionCur, previousFrame[i].m_worldMatrix);
		XMMatrixDecompose(&scaleNext, &rotationNext, &positionNext, nextFrame[i].m_worldMatrix);
		XMVECTOR rotNow = XMQuaternionSlerp(rotationCur, rotationNext, ratio);
		XMVECTOR scaleNow = XMVectorLerp(scaleCur, scaleNext, ratio);
		XMVECTOR posNow = XMVectorLerp(positionCur, positionNext, ratio);
		XMMATRIX MatrixNow = XMMatrixAffineTransformation(scaleNow, XMVectorZero(), rotNow, posNow);
		trns.m_worldMatrix = MatrixNow;
		ret.push_back(trns);
	}

	return ret; //Fill in before returning;
}