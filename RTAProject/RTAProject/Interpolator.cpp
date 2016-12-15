#include "Interpolator.h"
#include <DirectXMath.h>

using namespace DirectX;

Interpolator::Interpolator()
{
}

Interpolator::~Interpolator()
{
}

void Interpolator::Process(float _timeToAdd) {
	frameTime += _timeToAdd;
	while(frameTime > animationPtr->m_keyFrame[nextFrame].GetFrameTime()) {
		previousFrame = nextFrame;
		frameTime -= animationPtr->m_keyFrame[nextFrame++].GetFrameTime();
		// Please error check NextFrame result
	}
	float delta = frameTime / animationPtr->m_keyFrame[previousFrame].GetFrameTime(); // Value between 0.f and 1.f
	betweenKeyFrame = Interpolate(previousFrame, nextFrame, delta);
}

KeyFrame Interpolator::Interpolate(unsigned int previousFrame, unsigned int nextFrame, float delta){
	//Interpolating between two key frame using XMMath…
	
	XMVECTOR scaleCur, rotationCur, positionCur, scaleNext, rotationNext, positionNext;
	KeyFrame ret;
	Transform trns; //Fill in before getting it into ret
	float ratio = 0.0f; //Need to calculate

	//for each bone
	for (size_t i = 0; i < animationPtr->m_keyFrame.size(); i++)
	{
		XMMatrixDecompose(&scaleCur, &rotationCur, &positionCur, animationPtr->m_keyFrame[i].m_bones[animationPtr->m_keyFrame[previousFrame].GetBoneIndex()].m_worldMatrix);
		XMMatrixDecompose(&scaleNext, &rotationNext, &positionNext, animationPtr->m_keyFrame[i].m_bones[animationPtr->m_keyFrame[nextFrame].GetBoneIndex()].m_worldMatrix);
		XMVECTOR rotNow = XMQuaternionSlerp(rotationCur, rotationNext, ratio);
		XMVECTOR scaleNow = XMVectorLerp(scaleCur, scaleNext, ratio);
		XMVECTOR posNow = XMVectorLerp(positionCur, positionNext, ratio);
		XMMATRIX MatrixNow = XMMatrixAffineTransformation(scaleNow, XMVectorZero(), rotNow, posNow);
		trns.m_worldMatrix = MatrixNow;
		ret.m_bones.push_back(trns);
	}

	return ret; //Fill in before returning;
}