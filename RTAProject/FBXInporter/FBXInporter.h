#pragma once  
#include "fbxsdk.h"
#include <DirectXMath.h>
#include <vector>

#ifdef FBXImorter_EXPORTS  
#define FBXImporter_API __declspec(dllexport)   
#else  
#define FBXImporter_API __declspec(dllimport)   
#endif  

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


	class Functions
	{
		static void TraverseScene(FbxNode* _node, vector<VERTEX>& _vertecies,vector<int>& _indices, vector<TRANSFORM_NODE>& _transformHierarchy);
		static int GetDataFromMesh(FbxNode* inNode, vector<VERTEX>& _vertecies, vector<int>& _indices);
		//static void GetDataFromSkeleton(FbxNode* _inNode, std::vector<TRANSFORM_NODE>& _transformHierarchy);		
	public:
		static FBXImporter_API int LoadFBXFile(const string & _fileName, vector<VERTEX>& _vertecies, vector<int>& _indices, vector<TRANSFORM_NODE>& _transformHierarchy);

	};

}