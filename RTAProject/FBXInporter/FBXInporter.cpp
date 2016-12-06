// FBXInporter.cpp : Defines the exported functions for the DLL application.
#include "stdafx.h"  
#include "FBXInporter.h"
#include "FBXLib.h"
#include <fstream>

namespace FBXImporter
{
	// The only function that is being called outside
	int LoadFBXFile(const string & _fileName, vector<VERTEX>& _vertecies, vector<unsigned int>& _indices, vector<Transform>& _transformHierarchy)
	{
		// Change the following filename to a suitable filename value.
		const char* lFilename = _fileName.c_str();

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
		FbxScene* fbxScene = FbxScene::Create(lSdkManager, "myScene");

		// Import the contents of the file into the scene.
		lImporter->Import(fbxScene);

		// The file is imported, so get rid of the importer.
		lImporter->Destroy();

		FbxNode *root = fbxScene->GetRootNode();

		TraverseScene(root, _vertecies, _indices, _transformHierarchy);
		ExportBinaryFile(_fileName, _vertecies, _indices);
		ExportBinaryFile(_fileName, _transformHierarchy);

		fbxScene->Destroy();
		lSdkManager->Destroy();
		//vector<KEYFRAME_DATA> temp;
		//GetFrameData(fbxScene,temp);

		return 0;
	}

	// Traverses the scene object
	void TraverseScene(FbxNode* _node, vector<VERTEX>& _vertecies, vector<unsigned int>& _indices, vector<Transform>& _transformHierarchy)
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
					GetDataFromMesh(child, _vertecies, _indices, _transformHierarchy);
				//else if (child->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eSkeleton)
					//	GetDataFromSkeleton(child, _transformHierarchy);
			}
		}
	}

	// Gets the verticies data from the mesh
	void GetDataFromMesh(FbxNode* _inNode, vector<VERTEX>& _vertecies, vector<unsigned int>& _indicies, std::vector<Transform>& _transformHierarchy)
	{

		FbxMesh* currMesh = _inNode->GetMesh();
		//currMesh->GetDeformer()
		LoadMeshSkeleton(currMesh, _transformHierarchy);
		//Containers
		vector<VERTEX> controlPointsList;

		int triangleCount = currMesh->GetPolygonCount();
		for (int triIndex = 0; triIndex < triangleCount; ++triIndex)
		{
			for (int triVertIndex = 0; triVertIndex < 3; ++triVertIndex)
			{
				int ctrlPointIndex = currMesh->GetPolygonVertex(triIndex, triVertIndex);
				_indicies.push_back(ctrlPointIndex);
			}
		}

		for (int j = 0; j < currMesh->GetControlPointsCount(); ++j)
		{
			// Setting Vertecies
			VERTEX currVertex;
			currVertex.transform = XMFLOAT3(float(currMesh->GetControlPointAt(j).mData[0]), float(currMesh->GetControlPointAt(j).mData[1]), float(currMesh->GetControlPointAt(j).mData[2]));
			//FbxVector4 tangents = currMesh->GetElementTangent(j)->GetDirectArray().GetAt(j);
			//FbxVector4 bitangents = currMesh->GetElementBinormal(j)->GetDirectArray().GetAt(j);
			//currVertex.tangents = { (float)tangents[0],(float)tangents[1],(float)tangents[2] };
			//currVertex.bitangents = { (float)bitangents[0],(float)bitangents[1],(float)bitangents[2] };
			controlPointsList.push_back(currVertex);
		}

		// Loop for each poly
		for (int polyIndex = 0; polyIndex < currMesh->GetPolygonCount(); ++polyIndex)
		{
			// Get number of verts in this poly
			const int NumVertices = currMesh->GetPolygonSize(polyIndex);

			// Loop for each vert in poly
			for (int Vertex = 0; Vertex < NumVertices; Vertex++)
			{
				FbxVector2 fbxTexCoord;
				FbxVector4 fbxNormals;
				FbxVector4 fbxTangents;
				FbxStringList UVSetNameList;
				VERTEX currVertex;

				// Get the name of each set of UV coords
				currMesh->GetUVSetNames(UVSetNameList);

				// Get data from mesh
				bool map;
				currMesh->GetPolygonVertexUV(polyIndex, Vertex, UVSetNameList.GetStringAt(0), fbxTexCoord, map);
				currMesh->GetPolygonVertexNormal(polyIndex, Vertex, fbxNormals);


				// Set the current vertex
				currVertex = controlPointsList[_indicies[polyIndex * 3 + Vertex]];
				currVertex.normals = { (float)fbxNormals.mData[0],(float)fbxNormals.mData[1] ,(float)fbxNormals.mData[2] };
				currVertex.uv = { float(fbxTexCoord[0]),float(1.0f - fbxTexCoord[1]),0 };

				// Store Data
				_vertecies.push_back(currVertex);
				_indicies[polyIndex * 3 + Vertex] = polyIndex * 3 + Vertex;
			}
		}
	}

	// Gets the frame data
	void GetFrameData(FbxScene* _inScene, std::vector<KEYFRAME_DATA>& _frameData)
	{
		int numAnimStacks = _inScene->GetSrcObjectCount<FbxAnimStack>();
		for (int animIndex = 0; animIndex < numAnimStacks; ++animIndex)
		{
			KEYFRAME_DATA currKey;
			FbxAnimStack* animStack = (FbxAnimStack*)_inScene->GetSrcObject<FbxAnimStack>(animIndex);

			// Getting the frame times
			FbxTimeSpan animTime = animStack->GetLocalTimeSpan();
			currKey.startTime = (float)animTime.GetStart().GetMilliSeconds();
			currKey.endTime = (float)animTime.GetStop().GetMilliSeconds();
			currKey.durationTime = (float)animTime.GetDuration().GetMilliSeconds();

			// Getting the bones positions
			int numFrames = animStack->GetMemberCount();
			for (int iFrame = 0; iFrame < numFrames; ++iFrame)
			{
				FbxObject *frame = animStack->GetMember(iFrame);
				FbxAnimLayer *animLayer = (FbxAnimLayer*)animStack->GetMember(iFrame);
				
				//animLayer->
				//FbxAnimCurve *translationCurve = fbxNode->LclTranslation.GetCurve(animLayer);
				//FbxAnimCurve *rotationCurve = fbxNode->LclRotation.GetCurve(animLayer);
				//FbxAnimCurve *scalingCurve = fbxNode->LclScaling.GetCurve(animLayer);

				//if (scalingCurve != 0)
				//{
				//	int numKeys = scalingCurve->KeyGetCount();
				//	for (int keyIndex = 0; keyIndex < numKeys; keyIndex++)
				//	{
				//		FbxTime frameTime = scalingCurve->KeyGetTime(keyIndex);
				//		FbxDouble3 scalingVector = fbxNode->EvaluateLocalScaling(frameTime);
				//		float x = (float)scalingVector[0];
				//		float y = (float)scalingVector[1];
				//		float z = (float)scalingVector[2];
				//
				//		float frameSeconds = (float)frameTime.GetSecondDouble(); // If needed, get the time of the scaling keyframe, in seconds
				//	}
				//}
				// else
				//{
				//	// If this animation layer has no scaling curve, then use the default one, if needed
				//	FbxDouble3 scalingVector = fbxNode->LclScaling.Get();
				//	float x = (float)scalingVector[0];
				//	float y = (float)scalingVector[1];
				//	float z = (float)scalingVector[2];
				//}			    
			}
			_frameData.push_back(currKey);
		}
	}

	// Loads the bind pose bones
	void LoadMeshSkeleton(FbxMesh *_inMesh, std::vector<Transform>& _transformHierarchy)
	{
		//int numDeformers = _inMesh->GetDeformerCount();
		vector<FbxNode*> bonesVector;
		FbxSkin* skin = (FbxSkin*)_inMesh->GetDeformer(0, FbxDeformer::eSkin);
		if (skin != 0)
		{
			int boneCount = skin->GetClusterCount();
			for (int boneIndex = 0; boneIndex < boneCount; boneIndex++)
			{
				Transform currBone;
				FbxCluster* cluster = skin->GetCluster(boneIndex);
				FbxNode* bone = cluster->GetLink();
				currBone.SetName(bone->GetName());
				
				// Get bone matrix
				FbxAMatrix wTransformMatrix = bone->EvaluateGlobalTransform();
				FbxAMatrix lTransformMatrix = bone->EvaluateLocalTransform();

				currBone.m_worldMatrix = CreateXMMatrixFromFBXVectors(wTransformMatrix.GetR(), wTransformMatrix.GetT(), wTransformMatrix.GetS());
				currBone.m_localMatrix = CreateXMMatrixFromFBXVectors(lTransformMatrix.GetR(), lTransformMatrix.GetT(), lTransformMatrix.GetS());

				/*
				int *boneVertexIndices = cluster->GetControlPointIndices();
				double *boneVertexWeights = cluster->GetControlPointWeights();
				// Iterate through all the vertices, which are affected by the bone
				int numBoneVertexIndices = cluster->GetControlPointIndicesCount();
				for (int boneVertexIndex = 0; boneVertexIndex < numBoneVertexIndices; boneVertexIndex++)
				{
					int boneVertIndex = boneVertexIndices[boneVertexIndex];
					float boneWeight = (float)boneVertexWeights[boneVertexIndex];
				}
				*/
				
				bonesVector.push_back(bone);
				_transformHierarchy.push_back(currBone);
			}

			SetBoneConnection(bonesVector, _transformHierarchy);
		}
	}

	// Exports fbx vertices data to a binary file
	void ExportBinaryFile(const string & _fileName, vector<VERTEX>& _vertecies, vector<unsigned int>& _indices)
	{
		ExporterHeader header(FileInfo::FILE_TYPES::MESH, _fileName.c_str());
		header.version = EXPORTER_VERSION_NUMBER;
		header.mesh.numPoints = _vertecies.size();
		header.mesh.numIndex = _indices.size();
		header.mesh.modelType = FileInfo::MODEL_TYPES::BASIC;
		header.mesh.vertSize = sizeof(VERTEX);
		header.mesh.index = FileInfo::INDEX_TYPES::TRI_STRIP;

		string binName;
		fstream binFile;
		const char* theName = strrchr(_fileName.c_str(), '\\');
		binName = strrchr(theName, theName[1]);
		binName.pop_back();
		binName.pop_back();
		binName.pop_back();
		binName.pop_back();
		binName.append("_mesh.bin");
		binFile.open(binName.c_str(), std::ios::out | std::ios::binary);
		if (binFile.is_open())
		{
			binFile.write((char*)&header, sizeof(FileInfo::ExporterHeader));
			for (size_t i = 0; i < _vertecies.size(); i++)
			{
				VERTEX temp = _vertecies[i];
				binFile.write((char*)&_vertecies[i], sizeof(VERTEX));
			}
			for (size_t i = 0; i < _indices.size(); i++)
			{
				unsigned int temp = _indices[i];
				binFile.write((char*)&_indices[i], sizeof(unsigned int));
			}
		}
		binFile.close();
	}

	// Exports fbx transform data 
	void ExportBinaryFile(const string & _fileName, vector<Transform> _bones)
	{
		ExporterHeader header(FileInfo::FILE_TYPES::BIND_POSE, _fileName.c_str());
		header.version = EXPORTER_VERSION_NUMBER;
		header.bind.numBones = _bones.size();

		string binName;
		fstream binFile;
		const char* theName = strrchr(_fileName.c_str(), '\\');
		binName = strrchr(theName, theName[1]);
		binName.pop_back();
		binName.pop_back();
		binName.pop_back();
		binName.pop_back();
		binName.append("_bindpose.bin");
		binFile.open(binName.c_str(), std::ios::out | std::ios::binary);
		if (binFile.is_open())
		{
			binFile.write((char*)&header, sizeof(FileInfo::ExporterHeader));
			for (size_t i = 0; i < _bones.size(); i++)
			{
				Transform temp = _bones[i];
				binFile.write((char*)&_bones[i], sizeof(Transform) * header.bind.numBones);
			}
		}
		binFile.close();
	}

	// Creates a XMMATRIX from fbx Vectors
	XMMATRIX CreateXMMatrixFromFBXVectors(FbxVector4 _rotVec, FbxVector4 _translVec, FbxVector4 _scaleVec)
	{
		XMMATRIX returnMatrix = XMMatrixIdentity();
		XMVECTOR sVec = { (float)_scaleVec.mData[0], (float)_scaleVec.mData[1], (float)_scaleVec.mData[2], (float)_scaleVec.mData[3] };
		XMVECTOR rVec = { (float)_rotVec.mData[0], (float)_rotVec.mData[1], (float)_rotVec.mData[2], (float)_rotVec.mData[3] };
		XMVECTOR tVec = { (float)_translVec.mData[0], (float)_translVec.mData[1], (float)_translVec.mData[2], (float)_translVec.mData[3] };
		returnMatrix = XMMatrixTranslationFromVector(tVec) * returnMatrix;
		returnMatrix = XMMatrixRotationRollPitchYawFromVector(rVec) * returnMatrix;
		returnMatrix = XMMatrixScalingFromVector(sVec) * returnMatrix;

		return returnMatrix;
	}

	// Creates the connection between bones
	void SetBoneConnection(vector<FbxNode*> _boneVect, std::vector<Transform>& _transformHierarchy)
	{
		for (unsigned int i = 0; i < _boneVect.size(); ++i)
		{
			FbxNode* parent = _boneVect[i]->GetParent();
			FbxNode* child = _boneVect[i]->GetChild(0);
			FbxNode* sibling = nullptr;
			Transform* childTN = nullptr, *siblingTN = nullptr, *parentTN = nullptr;

			// If this node is the root
			const char* CHECK_NAME = _boneVect[i]->GetName();
			if (_boneVect[i]->GetName()[0] == 'R')
			{
				if (child)
				{
					childTN = &CheckTransform(_transformHierarchy, child->GetName());
					SetTransformNode((*childTN), child);
					childTN->AddParent(&_transformHierarchy[i]);
					_transformHierarchy[i].AddChild(childTN);
				}
			}
			else
			{
				// Set parent
				parentTN = &CheckTransform(_transformHierarchy, parent->GetName());
				SetTransformNode((*parentTN), parent);
				_transformHierarchy[i].AddParent(parentTN);

				// Set the sibling
				if (parent->GetChildCount() > 1)
				{
					if (parent->GetChild(0)->GetName()[0] == _boneVect[i]->GetName()[0])
					{
						if (parent->GetChild(1))
							sibling = parent->GetChild(1);
					}
					else
					{
						if (parent->GetChild(0))
							sibling = parent->GetChild(0);
					}
					siblingTN = &CheckTransform(_transformHierarchy, sibling->GetName());
					SetTransformNode((*siblingTN), sibling);
					_transformHierarchy[i].AddSibling(siblingTN);
				}

				// Set child
				if (child)
				{
					childTN = &CheckTransform(_transformHierarchy, child->GetName());
					SetTransformNode((*childTN), child);
					childTN->AddParent(&_transformHierarchy[i]);
					_transformHierarchy[i].AddChild(childTN);
				}
			}
		}
	}

	// Helper for making the connection between bones
	void SetTransformNode(Transform& _transforms, FbxNode* _theNode)
	{
		FbxAMatrix wTransformMatrix = _theNode->EvaluateGlobalTransform();
		FbxAMatrix lTransformMatrix = _theNode->EvaluateLocalTransform();
		_transforms.m_worldMatrix = CreateXMMatrixFromFBXVectors(wTransformMatrix.GetR(), wTransformMatrix.GetT(), wTransformMatrix.GetS());
		_transforms.m_localMatrix = CreateXMMatrixFromFBXVectors(lTransformMatrix.GetR(), lTransformMatrix.GetT(), lTransformMatrix.GetS());
		_transforms.SetDirty(false);
		_transforms.SetName(_theNode->GetName());
	}

	// Helper for making the connection between bones
	Transform& CheckTransform(std::vector<Transform>& _transformHierarchy, const char* _id)
	{
		for (unsigned int i = 0; i < _transformHierarchy.size(); ++i)
		{
			if (_transformHierarchy[i].GetName() == _id)
				return _transformHierarchy[i];
		}
		return _transformHierarchy[0];
	}

}// FBXImporter namespace