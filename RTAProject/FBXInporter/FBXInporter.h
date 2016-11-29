#pragma once  

#include "fbxsdk.h"
#include <DirectXMath.h>
#include <vector>
// For smart pointers
#include <atlbase.h>

#ifdef FBXInporter_EXPORTS  
#define FBXInporter_API __declspec(dllexport)   
#else  
#define FBXInporter_API __declspec(dllimport)   
#endif  

using namespace DirectX;

namespace FBXInporter
{
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

	FbxScene * n_fbxScene;

	class Functions
	{
		static void TraverseScene(FbxNode* _node, std::vector<VERTEX>& _vertecies, std::vector<int>& _indices, std::vector<TRANSFORM_NODE>& _transformHierarchy);
		static int GetDataFromMesh(FbxNode* inNode, std::vector<VERTEX>& _vertecies, std::vector<int>& _indices);
		//static void GetDataFromSkeleton(FbxNode* _inNode, std::vector<TRANSFORM_NODE>& _transformHierarchy);		
	public:
		static FBXInporter_API int LoadFBXFile(const std::string & _fileName, std::vector<VERTEX>& _vertecies, std::vector<int>& _indices, std::vector<TRANSFORM_NODE>& _transformHierarchy);

	};

}