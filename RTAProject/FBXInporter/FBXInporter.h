#pragma once  
#include "fbxsdk.h"
#include <DirectXMath.h>
#include <vector>
#include "ExporterHeader.h"
#include "Transform.h"
#include "Animation.h"
#include "KeyFrame.h"

using namespace FileInfo;
using namespace DirectX;
using namespace std;

namespace FBXImporter
{
	FbxScene* fbxScene;

	struct VERTEX
	{
		XMFLOAT3 transform;
		XMFLOAT3 normals;
		XMFLOAT3 uv;
		XMFLOAT3 tangents;
		XMFLOAT3 bitangents;
		XMFLOAT3 shine;
	};

	
	void TraverseScene(FbxNode* _node, vector<VERTEX>& _vertecies, vector<unsigned int>& _indices, vector<Transform>& _transformHierarchy, Animation& _animation);
	void GetDataFromMesh(FbxNode* inNode, vector<VERTEX>& _vertecies, vector<unsigned int>& _indices, std::vector<Transform>& _transformHierarchy, Animation& _animation);
	void GetAnimationData(FbxScene* _inScene, FbxNode* _inNode, Animation& _animation);

	// Overloaded Export to Binary Functions
	void ExportBinaryFile(const string & _fileName, vector<VERTEX>& _vertecies, vector<unsigned int>& _indices);
	void ExportBinaryFile(const string & _fileName, vector<Transform>& _bones);
	void ExportBinaryFile(const string & _fileName, Animation& _animation);

	void LoadMeshSkeleton(FbxMesh *_inMesh, std::vector<Transform>& _transformHierarchy, Animation& _animation);
	XMMATRIX CreateXMMatrixFromFBXVectors(FbxVector4 _rotVec, FbxVector4 _translVec, FbxVector4 _scaleVec);
	void SetBoneConnection(vector<FbxNode*> _boneVect, std::vector<Transform>& _transformHierarchy);
	void SetTransformNode(Transform& _transforms, FbxNode* _theNode);
	Transform& CheckTransform(std::vector<Transform>& _transformHierarchy, const char* _id);


}
