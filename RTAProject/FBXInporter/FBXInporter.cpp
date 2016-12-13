// FBXInporter.cpp : Defines the exported functions for the DLL application.
#include "stdafx.h"  
#include "FBXInporter.h"
#include "FBXLib.h"
#include <fstream>

namespace FBXImporter
{
	// The only function that is being called outside
	int LoadFBXFile(const string & _fileName, vector<VERTEX>& _vertecies, vector<unsigned int>& _indices, vector<Transform>& _transformHierarchy, Animation& _animation)
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
	    fbxScene = FbxScene::Create(lSdkManager, "myScene");

		// Import the contents of the file into the scene.
		lImporter->Import(fbxScene);

		// The file is imported, so get rid of the importer.
		lImporter->Destroy();

		FbxNode *root = fbxScene->GetRootNode();

		TraverseScene(root, _vertecies, _indices, _transformHierarchy, _animation);

		// Containers for OBJ
		ExportBinaryFile(_fileName, _vertecies, _indices);
		ExportBinaryFile(_fileName, _transformHierarchy);
		ExportBinaryFile(_fileName, _animation);

		fbxScene->Destroy();
		lSdkManager->Destroy();
		
		return 0;
	}

	// Traverses the scene object
	void TraverseScene(FbxNode* _node, vector<VERTEX>& _vertecies, vector<unsigned int>& _indices, vector<Transform>& _transformHierarchy, Animation& _animation)
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
				TraverseScene(child, _vertecies, _indices, _transformHierarchy,_animation);
				if (child->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eMesh)
					GetDataFromMesh(child, _vertecies, _indices, _transformHierarchy,_animation);
				//else if (child->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eSkeleton)
					//	GetDataFromSkeleton(child, _transformHierarchy);
			}
		}
	}

	// Gets the verticies data from the mesh
	void GetDataFromMesh(FbxNode* _inNode, vector<VERTEX>& _vertecies, vector<unsigned int>& _indicies, std::vector<Transform>& _transformHierarchy, Animation& _animation)
	{

		FbxMesh* currMesh = _inNode->GetMesh();
		LoadMeshSkeleton(currMesh, _transformHierarchy, _animation);
		
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
			currVertex.transform = XMFLOAT4(float(currMesh->GetControlPointAt(j).mData[0]), float(currMesh->GetControlPointAt(j).mData[1]), float(currMesh->GetControlPointAt(j).mData[2]),0);
			controlPointsList.push_back(currVertex);
		}

		LoadMeshSkin(currMesh, controlPointsList);

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
				//currVertex.skin
				currVertex.normals = { (float)fbxNormals.mData[0],(float)fbxNormals.mData[1] ,(float)fbxNormals.mData[2],0 };
				currVertex.uv = { float(fbxTexCoord[0]),float(1.0f - fbxTexCoord[1]),0,0 };
				
				// Store Data
				_vertecies.push_back(currVertex);
				_indicies[polyIndex * 3 + Vertex] = polyIndex * 3 + Vertex;
			}
		}
	}

	// Gets the frame data
	void GetAnimationData(FbxScene* _inScene, FbxNode* _inNode, Animation& _animation)
	{
		int numAnimStacks = _inScene->GetSrcObjectCount<FbxAnimStack>();


		FbxAnimStack* animStack = (FbxAnimStack*)_inScene->GetSrcObject<FbxAnimStack>(0);
		const char* animStackName = animStack->GetName();

		// Getting the frame times
		FbxTimeSpan animTime = animStack->GetLocalTimeSpan();
		FbxTime startTime = animTime.GetStart();
		FbxTime endTime = animTime.GetStop();
		// Setting the lenght of the animation
		_animation.SetTotalTime(float(animTime.GetDuration().GetMilliSeconds()));

		unsigned int numFrames = (unsigned int)(endTime.GetFrameCount(FbxTime::eFrames30) - startTime.GetFrameCount(FbxTime::eFrames30) + 1);
		// Setting the num of frames
		_animation.SetKeyFramesNumber(numFrames);

		FbxAMatrix boneTransform = _inNode->EvaluateGlobalTransform();
		KeyFrame currFrame;
		// Getting data for each frame
		for (FbxLongLong iFrame = startTime.GetFrameCount(FbxTime::eFrames30); iFrame < endTime.GetFrameCount(FbxTime::eFrames30); ++iFrame)
		{

			FbxTime currTime;
			currTime.SetFrame(iFrame, FbxTime::eFrames30);
			currFrame.SetFrameTime(float(currTime.GetFramedTime(false).GetMilliSeconds()));
			// Set the number of the currFrame
			currFrame.SetBoneIndex((unsigned int)iFrame);
			FbxAMatrix currentTransformOffset = _inNode->EvaluateGlobalTransform(currTime) * boneTransform;

			// Get bone matrix
			Transform currBone;
			FbxAMatrix wTransformMatrix = /*currentTransformOffset.Inverse() * */_inNode->EvaluateGlobalTransform(currTime);
			FbxAMatrix lTransformMatrix = _inNode->EvaluateLocalTransform();
			currBone.m_worldMatrix = CreateXMMatrixFromFBXVectors(wTransformMatrix.GetR(), wTransformMatrix.GetT(), wTransformMatrix.GetS());
			currBone.m_localMatrix = CreateXMMatrixFromFBXVectors(lTransformMatrix.GetR(), lTransformMatrix.GetT(), lTransformMatrix.GetS());

			currFrame.m_bones.push_back(currBone);
		}
		_animation.m_keyFrame.push_back(currFrame);
	}

	void LoadMeshSkeleton(FbxMesh *_inMesh, std::vector<Transform>& _transformHierarchy, Animation& _animation)
	{
		// Loads the bind pose bones
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

				/*//Skin stuff
				int *boneVertexIndices = cluster->GetControlPointIndices();
				double *boneVertexWeights = cluster->GetControlPointWeights();
				// Iterate through all the vertices, which are affected by the bone
				int numBoneVertexIndices = cluster->GetControlPointIndicesCount();
				for (int boneVertexIndex = 0; boneVertexIndex < numBoneVertexIndices; boneVertexIndex++)
				{
					int boneVertIndex = boneVertexIndices[boneVertexIndex];
					float boneWeight = (float)boneVertexWeights[boneVertexIndex];
				}*/
				

				GetAnimationData(fbxScene, bone, _animation);
				bonesVector.push_back(bone);
				_transformHierarchy.push_back(currBone);
			}
			
			SetBoneConnection(bonesVector, _transformHierarchy);
		}
	}

	void LoadMeshSkin(FbxMesh *_inMesh, vector<VERTEX>& _vertecies) 
	{
		FbxSkin* skin = (FbxSkin*)_inMesh->GetDeformer(0, FbxDeformer::eSkin);
		if (skin != 0)
		{
			// Initializing for debugging purposes
			unsigned int amountOfVertecies = _vertecies.size();
			TEMP_SKIN_DATA * tempSkin = new TEMP_SKIN_DATA[amountOfVertecies];
			for (unsigned int i = 0; i < amountOfVertecies; i++)
			{
				tempSkin[i].bonesStored = 0;
				for (unsigned int j = 0; j < 4; j++)
				{
					tempSkin[i].indices[j] = 0;
					tempSkin[i].weights[j] = 0;
				}
			}


			int boneCount = skin->GetClusterCount();
			for (int boneIndex = 0; boneIndex < boneCount; boneIndex++)
			{
				Transform currBone;
				FbxCluster* cluster = skin->GetCluster(boneIndex);

				//Skin stuff
				int *boneVertexIndices = cluster->GetControlPointIndices();
				double *boneVertexWeights = cluster->GetControlPointWeights();
				// Iterate through all the vertices, which are affected by the bone
				int numBoneVertexIndices = cluster->GetControlPointIndicesCount();
				for (int boneVertexIndex = 0; boneVertexIndex < numBoneVertexIndices; boneVertexIndex++)
				{
					int boneVertIndex = boneVertexIndices[boneVertexIndex];
					float boneWeight = (float)boneVertexWeights[boneVertexIndex];
					if (tempSkin[boneVertIndex].bonesStored < 4)
					{
						tempSkin[boneVertIndex].indices[tempSkin[boneVertIndex].bonesStored] = boneIndex;
						tempSkin[boneVertIndex].weights[tempSkin[boneVertIndex].bonesStored]= boneWeight;
						tempSkin[boneVertIndex].bonesStored += 1;
					}
				}
			}

			for (unsigned int i = 0; i < amountOfVertecies; i++)
			{
				_vertecies[i].skinIndices.x = (float)tempSkin[i].indices[0];
				_vertecies[i].skinIndices.y = (float)tempSkin[i].indices[1];
				_vertecies[i].skinIndices.z = (float)tempSkin[i].indices[2];
				_vertecies[i].skinIndices.w = (float)tempSkin[i].indices[3];

				_vertecies[i].skinWeights.x = tempSkin[i].weights[0];
				_vertecies[i].skinWeights.y = tempSkin[i].weights[1];
				_vertecies[i].skinWeights.z = tempSkin[i].weights[2];
				_vertecies[i].skinWeights.w = tempSkin[i].weights[3];
			}
			delete[] tempSkin;
			int a = 0;
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

	void ExportBinaryFile(const string & _fileName, vector<Transform>& _bones)
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
			binFile.write((char*)&_bones[0], sizeof(Transform) * header.bind.numBones);
		}
		binFile.close();
	}

	void ExportBinaryFile(const string & _fileName, Animation& _animation)
	{
		ExporterHeader header(FileInfo::FILE_TYPES::ANIMATION, _fileName.c_str());
		header.version = EXPORTER_VERSION_NUMBER;
		header.anim.endTime = _animation.GetTotalTime();
		header.anim.startTime = 0.0f;
		header.anim.numBones = _animation.m_keyFrame[0].GetBoneIndex() + 1;
		header.anim.numFrames = _animation.m_keyFrame.size();

		string binName;
		fstream binFile;
		const char* theName = strrchr(_fileName.c_str(), '\\');
		binName = strrchr(theName, theName[1]);
		binName.pop_back();
		binName.pop_back();
		binName.pop_back();
		binName.pop_back();
		binName.append("_anim.bin");
		binFile.open(binName.c_str(), std::ios::out | std::ios::binary);
		if (binFile.is_open())
		{
			binFile.write((char*)&header, sizeof(FileInfo::ExporterHeader));
			binFile.write((char*)&_animation, sizeof(Animation) - sizeof(vector<KeyFrame>));
			for (size_t i = 0; i < header.anim.numFrames; i++)
			{
				binFile.write((char*)&_animation.m_keyFrame[i], sizeof(KeyFrame) - sizeof(vector<Transform>));
				for (size_t j = 0; j < header.anim.numBones; j++)
				{
					binFile.write((char*)&_animation.m_keyFrame[i].m_bones[j], sizeof(Transform));
				}
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

	/*
	Reads vertecies, uvs, normals and other object stuff from a .obj file
	Followed http://www.opengl-tutorial.org/beginners-tutorials/tutorial-7-model-loading/ tutorial
	*/
	bool ExportObject(string filePath,float _shine)
	{
		// Local Variables
		vector<unsigned int> vertexIndices, uvIndices, normalIndices;
		vector<XMFLOAT4> temp_vertices;
		vector<XMFLOAT4> temp_normals;
		vector<XMFLOAT4> temp_uvs;

		FILE *file;

		// Opening file - "r" -> read in
		fopen_s(&file, filePath.c_str(), "r");
		// Check if file opened
		if (file == NULL)
			return false;

		// Looping until the file finishes
		while (true)
		{
			// Read the first word of the line
			char lineHeader[128];
			int res = fscanf_s(file, "%s", lineHeader, _countof(lineHeader));

			// Check if the file has finished
			if (res == EOF)
				break;

			// Check if the line is a vertex
			if (strcmp(lineHeader, "v") == 0)
			{
				XMFLOAT4 vertex;
				fscanf_s(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
				temp_vertices.push_back(vertex);
			}
			// Check if the line is a UV
			else if (strcmp(lineHeader, "vt") == 0)
			{
				XMFLOAT4 uv;
				fscanf_s(file, "%f %f\n", &uv.x, &uv.y);
				temp_uvs.push_back(uv);
			}
			// Check if the line is a normal
			else if (strcmp(lineHeader, "vn") == 0)
			{
				XMFLOAT4 normal;
				fscanf_s(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
				temp_normals.push_back(normal);
			}
			// Check if the line is a index
			else if (strcmp(lineHeader, "f") == 0)
			{
				std::string vertex1, vertex2, vertex3;
				unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
				int matches = fscanf_s(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
				if (matches != 9)
					return false;

				// Setting the indicies for the vertecies
				vertexIndices.push_back(vertexIndex[0] - 1);
				vertexIndices.push_back(vertexIndex[1] - 1);
				vertexIndices.push_back(vertexIndex[2] - 1);

				// Setting the indicies for the UVs
				uvIndices.push_back(uvIndex[0] - 1);
				uvIndices.push_back(uvIndex[1] - 1);
				uvIndices.push_back(uvIndex[2] - 1);

				// Setting the indicies for the normals
				normalIndices.push_back(normalIndex[0] - 1);
				normalIndices.push_back(normalIndex[1] - 1);
				normalIndices.push_back(normalIndex[2] - 1);
			}
		}

		// Setting vertecies member
		unsigned int m_numVerts = vertexIndices.size();
		VERTEX * m_vertecies = new VERTEX[m_numVerts];
		for (unsigned int i = 0; i < m_numVerts; ++i)
		{
			// Setting vertecies
			m_vertecies[i].transform = temp_vertices[vertexIndices[i]];
			// Setting normals
			m_vertecies[i].normals = temp_normals[normalIndices[i]];
			// Setting UVs
			m_vertecies[i].uv.x = temp_uvs[uvIndices[i]].x;
			m_vertecies[i].uv.y = temp_uvs[uvIndices[i]].y;
			m_vertecies[i].uv.z = _shine;
		}

		// Setting indecies member
		unsigned int m_numIndicies = vertexIndices.size();
		unsigned int * m_indexList = new unsigned int[m_numIndicies];
		for (unsigned int i = 0; i < m_numIndicies; ++i)
			m_indexList[i] = i;


		// EXPORT THE DATA
		string binName;
		fstream binFile;
		string theName = strrchr(filePath.c_str(), '\\');
		theName.erase(0, 1);
		binName = theName;
		binName.pop_back();
		binName.pop_back();
		binName.pop_back();
		binName.pop_back();
		binName.append("_obj.bin");
		binFile.open(binName.c_str(), std::ios::out | std::ios::binary);
		if (binFile.is_open())
		{
			binFile.write((char*)&m_numVerts, sizeof(unsigned int));
			binFile.write((char*)&m_vertecies[0], sizeof(VERTEX) * m_numVerts);
			binFile.write((char*)&m_numIndicies, sizeof(unsigned int));
			binFile.write((char*)&m_indexList[0], sizeof(unsigned int) * m_numIndicies);
		}
		binFile.close();
		delete[] m_vertecies;
		delete[] m_indexList;
		// Return true if everything went right
		return true;
	}
}// FBXImporter namespace