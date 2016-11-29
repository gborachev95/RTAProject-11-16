// FBXInporter.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"  
#include "FBXInporter.h"
#include "fbxsdk.h"

using namespace std;


namespace FBXInporter
{
	/* [out] Test nmodels provided will only have one mesh, but other assets may have multiple
	meshes using the same rig to create a model
	[out] A container of all the joint transforms found. As these will all be in the same
	hierarchy, you may only need the root instead of a list of all nodes.*/
	int Functions::LoadFBXFile(const std::string & _fileName, std::vector<VERTEX>& _vertecies, std::vector<int>& _indices, std::vector<TRANSFORM_NODE>& _transformHierarchy)
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

	void Functions::TraverseScene(FbxNode* _node, std::vector<VERTEX>& _vertecies, std::vector<int>& _indices, std::vector<TRANSFORM_NODE>& _transformHierarchy)
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

	int Functions::GetDataFromMesh(FbxNode* _inNode, std::vector<VERTEX>& _vertecies, std::vector<int>& _indicies)
	{

		FbxMesh* currMesh = _inNode->GetMesh();

		int triangleCount = currMesh->GetPolygonCount();
		int vertexCounter = 0;
		for (int triIndex = 0; triIndex < triangleCount; ++triIndex)
		{
			for (int triVertIndex = 0; triVertIndex < 3; ++triVertIndex)
			{
				int ctrlPointIndex = currMesh->GetPolygonVertex(triIndex, triVertIndex);
				VERTEX currVertex;

				currVertex.transform = XMFLOAT3(float(currMesh->GetControlPointAt(ctrlPointIndex).mData[0]), float(currMesh->GetControlPointAt(ctrlPointIndex).mData[1]), float(currMesh->GetControlPointAt(ctrlPointIndex).mData[2]));
				//currVertex.uv = XMFLOAT3(currMesh->GetUV)
				//currVertex.normals = XMFLOAT3(currMesh->getnormal)

				_vertecies.push_back(currVertex);
				_indicies.push_back(ctrlPointIndex);
			}
		}
		return 0;
	}

	//void GetDataFromSkeleton(FbxNode* _inNode, std::vector<TRANSFORM_NODE>& _transformHierarchy)
	//{
	////	FbxSkeleton* currSkeleton = _inNode->GetSkeleton();
	//
	//	//currSkeleton-
	//	
	//}
}