// FBXInporter.cpp : Defines the exported functions for the DLL application.
#include "stdafx.h"  
#include "FBXInporter.h"
#include "FBXLib.h"

namespace FBXImporter
{
	/* [out] Test nmodels provided will only have one mesh, but other assets may have multiple
	meshes using the same rig to create a model
	[out] A container of all the joint transforms found. As these will all be in the same
	hierarchy, you may only need the root instead of a list of all nodes.*/
	int LoadFBXFile(const string & _fileName, vector<VERTEX>& _vertecies, vector<unsigned int>& _indices, vector<TRANSFORM_NODE>& _transformHierarchy)
	{
		// Change the following filename to a suitable filename value.
		const char* lFilename = _fileName.c_str();//"file.fbx";

												  // Initialize the SDK manager. This object handles memory management.
		FbxManager* lSdkManager = FbxManager::Create();

		/*To import the contents of an FBX file, a FbxIOSettings object and a FbxImporter object must be created.
		A FbxImporter object is initialized by providing the filename of the file to import along with a
		FbxIOSettings object that has been appropriately configured to suit the importing needs(see I / O Settings).*/

		// Create the IO settings object.
		FbxIOSettings *ios = FbxIOSettings::Create(lSdkManager, IOSROOT);
		lSdkManager->SetIOSettings(ios);

		// Create anmk importer using the SDK manager.
		FbxImporter* lImporter = FbxImporter::Create(lSdkManager, "");

		// Use the first argument as the filename for the importer.
		if (!lImporter->Initialize(lFilename, -1, lSdkManager->GetIOSettings()))
		{
			printf("Call to FbxImporter::Initialize() failed.\n");
			printf("Error returned: %s\n\n", lImporter->GetStatus().GetErrorString());
			exit(-1);
		}

		/*The FbxImporter object populates a provided FbxScene object with the elements contained in the FBX file.
		Observe that an empty string is passed as the second parameter in the FbxScene::Create() function.
		Objects created in the FBX SDK can be given arbitrary, non - unique names, that allow the user
		or other programs to identify the object after it is exported.After the FbxScene is populated,
		the FbxImporter can be safely destroyed.*/

		// Create a new scene so that it can be populated by the imported file.
		n_fbxScene = FbxScene::Create(lSdkManager, "myScene");

		// Import the contents of the file into the scene.
		lImporter->Import(n_fbxScene);

		// The file is imported, so get rid of the importer.
		lImporter->Destroy();


		FbxNode *root = n_fbxScene->GetRootNode();
		
		TraverseScene(root, _vertecies, _indices, _transformHierarchy);

		return 0;
	}

	void TraverseScene(FbxNode* _node, vector<VERTEX>& _vertecies, vector<unsigned int>& _indices, vector<TRANSFORM_NODE>& _transformHierarchy)
	{
		int childCount = 0;
		// Exit Contidion
		if (_node->GetChildCount() == 0)
			return;
		else
		{
			childCount = _node->GetChildCount();

			for (int i = 0; i < childCount; ++i)
			{
				FbxNode * child = _node->GetChild(i);
				TraverseScene(child, _vertecies, _indices, _transformHierarchy);
				if (child->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eMesh)
					GetDataFromMesh(child, _vertecies, _indices);
				if (child->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eSkeleton)
				{
					//	GetDataFromSkeleton(child, _transformHierarchy);
				}
			}
		}
	}

	int GetDataFromMesh(FbxNode* _inNode, vector<VERTEX>& _vertecies, vector<unsigned int>& _indicies)
	{

		FbxMesh* currMesh = _inNode->GetMesh();

		//int triangleCount = currMesh->GetPolygonCount();
		//int vertexCounter = 0;
		//for (int triIndex = 0; triIndex < triangleCount; ++triIndex)
		//{
		//	for (int triVertIndex = 0; triVertIndex < 3; ++triVertIndex)
		//	{
		//		int ctrlPointIndex = currMesh->GetPolygonVertex(triIndex, triVertIndex);
		//		VERTEX currVertex;
		//
		//		currVertex.transform = XMFLOAT3(float(currMesh->GetControlPointAt(ctrlPointIndex).mData[0]), float(currMesh->GetControlPointAt(ctrlPointIndex).mData[1]), float(currMesh->GetControlPointAt(ctrlPointIndex).mData[2]));
		//
		//		_vertecies.push_back(currVertex);
		//		_indicies.push_back(currMesh->GetPolygonVertexIndex(ctrlPointIndex));
		//	}
		//}

		// Getting the Vertecies DO NOT DELETE
		//int numVertices = currMesh->GetControlPointsCount();
		//FbxVector4 *verticesFbx = currMesh->GetControlPoints();
		//FbxGeometryElementNormal* normalsElement = currMesh->GetElementNormal();
		//FbxLayerElementArrayTemplate<FbxVector2>* uvVertices = 0;
		//currMesh->GetTextureUV(&uvVertices, FbxLayerElement::eTextureDiffuse);
		//
		//for (int vertexIndex = 0; vertexIndex < numVertices; ++vertexIndex)
		//{
		//	VERTEX currVertex;
		//	currVertex.transform = XMFLOAT3((float)verticesFbx[vertexIndex][0], (float)verticesFbx[vertexIndex][1], (float)verticesFbx[vertexIndex][2]);
		//
		//	// Getting normals 
		//	FbxVector4 normal = normalsElement->GetDirectArray().GetAt(vertexIndex);
		//	currVertex.normals = XMFLOAT3((float)normal[0], (float)normal[1], (float)normal[2]);
		//
		//
		//	// Getting the UVs
		//	FbxVector2 uv = (*uvVertices)[vertexIndex*5];
		//	currVertex.uv.x = (float)uv[0];
		//	currVertex.uv.y = (float)uv[1];
		//	currVertex.uv.z = 0.0f;
		//
		//	// Set the Vertex
		//	_vertecies.push_back(currVertex);
		//}
		// Getting the Indices
		int triangleCount = currMesh->GetPolygonCount();
		for (int triIndex = 0; triIndex < triangleCount; ++triIndex)
		{
			for (int triVertIndex = 0; triVertIndex < 3; ++triVertIndex)
			{
				int ctrlPointIndex = currMesh->GetPolygonVertex(triIndex, triVertIndex);
				_indicies.push_back(ctrlPointIndex);
			}
		}

		// Getting Normals
		//vector<XMFLOAT3> normals;
		//FbxGeometryElementNormal* normalEl = currMesh->GetElementNormal();
		//if (normalEl)
		//{
		//	int numNormals = currMesh->GetPolygonCount() * 3;
		//	int vertexCounter = 0;
		//	for (int polyCounter = 0; polyCounter<currMesh->GetPolygonCount(); polyCounter++)
		//	{
		//		for (int i = 0; i<3; i++)
		//		{
		//			XMFLOAT3 currNormal;
		//			FbxVector4 normal = normalEl->GetDirectArray().GetAt(vertexCounter);
		//			currNormal.x = (float)normal[0];
		//			currNormal.y = (float)normal[1];
		//			currNormal.z = (float)normal[2];
		//
		//			normals.push_back(currNormal);
		//			vertexCounter++;
		//		}
		//	}
		//}

		int vertexCounter = 0;
		fbxsdk::FbxGeometryElementNormal* normalEl = currMesh->GetElementNormal();
		int polygonCount = currMesh->GetPolygonCount();
		for (int i = 0; i < polygonCount; ++i) 
		{
			FbxLayerElementArrayTemplate<fbxsdk::FbxVector2>* uvVertices = 0;
			currMesh->GetTextureUV(&uvVertices, fbxsdk::FbxLayerElement::eTextureDiffuse);
			for (int j = 0; j < currMesh->GetPolygonSize(i); ++j) 
			{
				// Setting Vertecies
				VERTEX currVertex;
				currVertex.transform = XMFLOAT3(float(currMesh->GetControlPointAt(vertexCounter).mData[0]), float(currMesh->GetControlPointAt(vertexCounter).mData[1]), float(currMesh->GetControlPointAt(vertexCounter).mData[2]));

				// Setting Normals
				FbxVector4 normal = normalEl->GetDirectArray().GetAt(vertexCounter);
				currVertex.normals.x = (float)normal[0];
				currVertex.normals.y = (float)normal[1];
				currVertex.normals.z = (float)normal[2];
				

				// Setting UVs
				currVertex.uv.x = (float)(*uvVertices)[vertexCounter].mData[0];
				currVertex.uv.y = (float)(*uvVertices)[vertexCounter].mData[1];

				++vertexCounter;

				// Pushing the final result
				_vertecies.push_back(currVertex);
			}

		}

		return 0;
	}

	//void GetDataFromSkeleton(FbxNode* _inNode, std::vector<TRANSFORM_NODE>& _transformHierarchy)
	//{
	//int numDeformers = fbxMesh->GetDeformerCount();
	//FbxSkin* skin = (FbxSkin*)fbxMesh->GetDeformer(0, FbxDeformer::eSkin);
	//if (skin != 0)
	//{
	//	int boneCount = skin->GetClusterCount();
	//	for (int boneIndex = 0; boneIndex < boneCount; boneIndex++)
	//	{
	//		FbxCluster* cluster = skin->GetCluster(boneIndex);
	//		FbxNode* bone = cluster->GetLink(); // Get a reference to the bone's node
	//
	//											// Get the bind pose
	//		FbxAMatrix bindPoseMatrix;
	//		cluster->GetTransformLinkMatrix(bindPoseMatrix);
	//
	//		int *boneVertexIndices = cluster->GetControlPointIndices();
	//		double *boneVertexWeights = cluster->GetControlPointWeights();
	//
	//		// Iterate through all the vertices, which are affected by the bone
	//		int numBoneVertexIndices = cluster->GetControlPointIndicesCount();
	//		for (int boneVertexIndex = 0; boneVertexIndex < numBoneVertexIndices; boneVertexIndex++)
	//		{
	//			int boneVertexIndex = boneVertexIndices[boneVertexIndex];
	//			float boneWeight = (float)boneVertexWeights[boneVertexIndex];
	//		}
	//	}
	//}
	//}
}