#pragma once  
#include "fbxsdk.h"
#include <DirectXMath.h>
#include <vector>


using namespace DirectX;
using namespace std;

namespace FBXImporter
{
	FbxScene * n_fbxScene;

	struct VERTEX
	{
		XMFLOAT3 transform;
		XMFLOAT3 normals;
		XMFLOAT3 uv;
		XMFLOAT3 tangents;
		XMFLOAT3 bitangents;
	};

	struct TRANSFORM_NODE
	{
		TRANSFORM_NODE* parent;
		XMMATRIX localMatrix;
		XMMATRIX worldMatrix;
	};

	struct TRIANGLE
	{
		VERTEX vertecies[3];
	};


	void TraverseScene(FbxNode* _node, vector<VERTEX>& _vertecies, vector<unsigned int>& _indices, vector<TRANSFORM_NODE>& _transformHierarchy);
	int GetDataFromMesh(FbxNode* inNode, vector<VERTEX>& _vertecies, vector<unsigned int>& _indices);
	//static void GetDataFromSkeleton(FbxNode* _inNode, std::vector<TRANSFORM_NODE>& _transformHierarchy);		 

}
