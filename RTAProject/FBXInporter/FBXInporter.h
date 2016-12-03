#pragma once  
#include "fbxsdk.h"
#include <DirectXMath.h>
#include <vector>


using namespace DirectX;
using namespace std;

namespace FBXImporter
{
	struct VERTEX
	{
		XMFLOAT3 transform;
		XMFLOAT3 normals;
		XMFLOAT3 uv;
		XMFLOAT3 tangents;
		XMFLOAT3 bitangents;
		XMFLOAT3 shine;
	};

	struct TRANSFORM_NODE
	{
		TRANSFORM_NODE* parent;
		XMMATRIX localMatrix;
		XMMATRIX worldMatrix;
	};

	struct KEYFRAME_DATA
	{
		//Time keyTime;
		//XMMATRIX bones[bonesNum];
	};
	
	struct ANIMATION_DATA
	{

	};

	void TraverseScene(FbxNode* _node, vector<VERTEX>& _vertecies, vector<unsigned int>& _indices, vector<TRANSFORM_NODE>& _transformHierarchy);
	int GetDataFromMesh(FbxNode* inNode, vector<VERTEX>& _vertecies, vector<unsigned int>& _indices);
	void GetDataFromSkeleton(FbxNode* _inNode, std::vector<TRANSFORM_NODE>& _transformHierarchy);	
	void GetFrameData(FbxNode* _inNode, std::vector<KEYFRAME_DATA>& _frameData);
	void GetAnimationData(FbxScene* _inScene, std::vector<ANIMATION_DATA> _animData);

}
