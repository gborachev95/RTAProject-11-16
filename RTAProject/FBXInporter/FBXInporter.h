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

	struct SKIN_DATA
	{
		XMFLOAT4 weights;
		XMINT4 indices;
	};

	struct VERTEX
	{
		XMFLOAT4 transform;
		XMFLOAT4 normals;
		XMFLOAT4 uv;
		XMFLOAT4 tangents;
		XMFLOAT4 bitangents;
		XMFLOAT4 skinIndices;
		XMFLOAT4 skinWeights;
	};

	struct TEMP_SKIN_DATA
	{
		float weights[4];
		int indices[4];
		int bonesStored;
	};
	
	void TraverseScene(FbxNode* _node, vector<VERTEX>& _vertecies, vector<unsigned int>& _indices, vector<Transform>& _transformHierarchy, Animation& _animation);
	void GetDataFromMesh(FbxNode* inNode, vector<VERTEX>& _vertecies, vector<unsigned int>& _indices, std::vector<Transform>& _transformHierarchy, Animation& _animation);
	void GetAnimationData(FbxScene* _inScene, FbxNode* _inNode, Animation& _animation);

	// Overloaded Export to Binary Functions
	void ExportBinaryFile(const string & _fileName, vector<VERTEX>& _vertecies, vector<unsigned int>& _indices);
	void ExportBinaryFile(const string & _fileName, vector<Transform>& _bones);
	void ExportBinaryFile(const string & _fileName, Animation& _animation);

	//bool ExportObject(string filePath);

	void LoadMeshSkeleton(FbxMesh *_inMesh, std::vector<Transform>& _transformHierarchy, Animation& _animation);
	void LoadMeshSkin(FbxMesh *_inMesh, vector<VERTEX>& _vertecies);
	XMMATRIX CreateXMMatrixFromFBXVectors(FbxVector4 _rotVec, FbxVector4 _translVec, FbxVector4 _scaleVec);
	void SetBoneConnection(vector<FbxNode*> _boneVect, std::vector<Transform>& _transformHierarchy);
	void SetTransformNode(Transform& _transforms, FbxNode* _theNode);
	Transform& CheckTransform(std::vector<Transform>& _transformHierarchy, const char* _id);
	void ConvertMatrix(FbxAMatrix& _inputMatrix, XMMATRIX& _load);




}
