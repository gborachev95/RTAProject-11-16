#pragma once  
#include "fbxsdk.h"
#include <DirectXMath.h>
#include <vector>
#include "ExporterHeader.h"

using namespace FileInfo;
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
		TRANSFORM_NODE *sibling;
		XMMATRIX localMatrix;
		XMMATRIX worldMatrix;
	};

	struct KEYFRAME_DATA
	{
		float startTime,endTime,durationTime;
		vector<XMMATRIX> bones;
	};


	

	void TraverseScene(FbxNode* _node, vector<VERTEX>& _vertecies, vector<unsigned int>& _indices, vector<TRANSFORM_NODE>& _transformHierarchy);
	void GetDataFromMesh(FbxNode* inNode, vector<VERTEX>& _vertecies, vector<unsigned int>& _indices, std::vector<TRANSFORM_NODE>& _transformHierarchy);
	void GetDataFromSkeleton(FbxNode* _inNode, std::vector<TRANSFORM_NODE>& _transformHierarchy);	
	void GetFrameData(FbxScene* _inScene, std::vector<KEYFRAME_DATA>& _frameData);
	void ExportBinaryFile(const string & _fileName, vector<VERTEX>& _vertecies, vector<unsigned int>& _indices);
	void LoadMeshSkeleton(FbxMesh *_inMesh, std::vector<TRANSFORM_NODE>& _transformHierarchy);
	XMMATRIX CreateXMMatrixFromFBXVectors(FbxVector4 _rotVec, FbxVector4 _translVec, FbxVector4 _scaleVec);


}
