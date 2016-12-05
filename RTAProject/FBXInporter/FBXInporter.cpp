// FBXInporter.cpp : Defines the exported functions for the DLL application.
#include "stdafx.h"  
#include "FBXInporter.h"
#include "FBXLib.h"
#include <fstream>

namespace FBXImporter
{
	/* 
	[out] Test nmodels provided will only have one mesh, but other assets may have multiple
	meshes using the same rig to create a model
	[out] A container of all the joint transforms found. As these will all be in the same
	hierarchy, you may only need the root instead of a list of all nodes.
	*/
	int LoadFBXFile(const string & _fileName, vector<VERTEX>& _vertecies, vector<unsigned int>& _indices, vector<TRANSFORM_NODE>& _transformHierarchy)
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

		vector<KEYFRAME_DATA> temp;
		GetFrameData(fbxScene, temp);

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
				{
					GetDataFromMesh(child, _vertecies, _indices);
					//GetDataFromSkeleton(child, _transformHierarchy);
				}
				else if (child->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eSkeleton)
						GetDataFromSkeleton(child, _transformHierarchy);
				//else if (child->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::e)
				//{
				//
				//}
			}
		}
	}

	int GetDataFromMesh(FbxNode* _inNode, vector<VERTEX>& _vertecies, vector<unsigned int>& _indicies)
	{

		FbxMesh* currMesh = _inNode->GetMesh();

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
				currVertex.normals = {(float)fbxNormals.mData[0],(float)fbxNormals.mData[1] ,(float)fbxNormals.mData[2] };
				currVertex.uv = { float(fbxTexCoord[0]),float(1.0f - fbxTexCoord[1]),0};
				
				// Store Data
				_vertecies.push_back(currVertex);
				_indicies[polyIndex * 3 + Vertex] = polyIndex * 3 + Vertex;
			}
		}
		return 0;
	}

	void GetDataFromSkeleton(FbxNode* _inNode, std::vector<TRANSFORM_NODE>& _transformHierarchy)
	{
		//int numDeformers = _inNode->GetDeformerCount();
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

		FbxSkeleton* currSkeleton = _inNode->GetSkeleton();	
		FbxSkeleton::EType type = currSkeleton->GetSkeletonType();
		int boneSize = currSkeleton->GetNodeCount();

		for (int i = 0; i < boneSize; ++i)
		{
			TRANSFORM_NODE tranformNode;
			FbxNode* bone = currSkeleton->GetNode(i);
			
			// Getting the local matrix
			FbxVector4 lRotationVector = bone->GetGeometricRotation(FbxNode::EPivotSet::eSourcePivot);
			FbxVector4 lScaleVector = bone->GetGeometricScaling(FbxNode::EPivotSet::eSourcePivot);
			FbxVector4 lTranslationVector = bone->GetGeometricTranslation(FbxNode::EPivotSet::eSourcePivot);
			XMMATRIX localMatrix = XMMatrixIdentity();
			XMVECTOR sVec = { (float)lScaleVector.mData[0], (float)lScaleVector.mData[1], (float)lScaleVector.mData[2], (float)lScaleVector.mData[3] };
			XMVECTOR rVec = { (float)lRotationVector.mData[0], (float)lRotationVector.mData[1], (float)lRotationVector.mData[2], (float)lRotationVector.mData[3] };
			XMVECTOR tVec = { (float)lTranslationVector.mData[0], (float)lTranslationVector.mData[1], (float)lTranslationVector.mData[2], (float)lTranslationVector.mData[3] };
			localMatrix = XMMatrixScalingFromVector(sVec) * localMatrix;
			localMatrix = XMMatrixRotationRollPitchYawFromVector(rVec) * localMatrix;
			localMatrix = XMMatrixTranslationFromVector(tVec) * localMatrix;

			// Getting the global matrix
			FbxVector4 wRotationVector = bone->GetGeometricRotation(FbxNode::EPivotSet::eDestinationPivot);
			FbxVector4 wScaleVector = bone->GetGeometricScaling(FbxNode::EPivotSet::eDestinationPivot);
			FbxVector4 wTranslationVector = bone->GetGeometricTranslation(FbxNode::EPivotSet::eDestinationPivot);
			XMMATRIX worldMatrix = XMMatrixIdentity();
			XMVECTOR wsVec = { (float)wScaleVector.mData[0], (float)wScaleVector.mData[1], (float)wScaleVector.mData[2], (float)wScaleVector.mData[3] };
			XMVECTOR wrVec = { (float)wRotationVector.mData[0], (float)wRotationVector.mData[1], (float)wRotationVector.mData[2], (float)wRotationVector.mData[3] };
			XMVECTOR wtVec = { (float)wTranslationVector.mData[0], (float)wTranslationVector.mData[1], (float)wTranslationVector.mData[2], (float)wTranslationVector.mData[3] };
			worldMatrix = XMMatrixScalingFromVector(wsVec) * worldMatrix;
			worldMatrix = XMMatrixRotationRollPitchYawFromVector(wrVec) * worldMatrix;
			worldMatrix = XMMatrixTranslationFromVector(wtVec) * worldMatrix;

			tranformNode.localMatrix = localMatrix;
			tranformNode.worldMatrix = worldMatrix;
			bone->GetParent();
			_transformHierarchy.push_back(tranformNode);
		}

		//FbxMesh* boneMesh = nullptr;
		//FbxNodeAttribute::EType type = currSkeleton->GetNode()->GetNodeAttribute()->GetAttributeType();
		//if ( type == currSkeleton->eMesh)
		//{
		//	boneMesh = currSkeleton->GetNode()->GetMesh();
		//
		//	FbxMesh* currMesh = boneMesh;
		//	int deformerCount = currMesh->GetDeformerCount(FbxDeformer::eSkin);
		//	int vertexCount = currMesh->GetControlPointsCount();
		//	for (int i = 0; i < deformerCount; ++i)
		//	{
		//		// Getting the skin
		//		FbxSkin *fbxSkin = (FbxSkin*)currMesh->GetDeformer(i, FbxDeformer::eSkin);
		//		if (fbxSkin == nullptr)
		//			continue;
		//
		//		// Bone count
		//		int bonesCount = fbxSkin->GetClusterCount();
		//
		//		// Iterate throuhg bones
		//		for (int boneIndex = 0; boneIndex < bonesCount; ++boneIndex)
		//		{
		//			// Cluster
		//			FbxCluster* cluster = fbxSkin->GetCluster(boneIndex);
		//
		//			// Bone reference
		//			FbxNode* bone = cluster->GetLink();
		//
		//			// Get the bind pose
		//			FbxAMatrix bindPoseMatrix, transformMatrix;
		//			cluster->GetTransformMatrix(transformMatrix);
		//			cluster->GetTransformLinkMatrix(bindPoseMatrix);
		//			XMMATRIX d311Matrix_bindPose, d311Matrix_transform;
		//
		//			// Decomposed transform components
		//			FbxVector4 scaleVector = bindPoseMatrix.GetS();
		//			FbxVector4 rotationVector = bindPoseMatrix.GetR();
		//			FbxVector4 translationVector = bindPoseMatrix.GetT();
		//
		//			int *vertexIndices = cluster->GetControlPointIndices();
		//			double *vertexWeights = cluster->GetControlPointWeights();
		//
		//			// Iterate through all the vertices, which are affected by the bone
		//			// Used for skinning
		//			//int vertexIndices = cluster->GetControlPointIndicesCount();
		//			//for (int iBoneVertexIndex = 0; iBoneVertexIndex < ncVertexIndices; iBoneVertexIndex++)
		//			//{
		//			//		// vertex
		//			//		int niVertex = pVertexIndices[iBoneVertexIndex];
		//			//
		//			//		// weight
		//			//		float fWeight = (float)pVertexWeights[iBoneVertexIndex];
		//			//}
		//
		//		} // Bones Inner for loop
		//	} // Deformer Outer for loop
		//
		//}
	} // Get the bones function

	void GetFrameData(FbxScene* _inScene, std::vector<KEYFRAME_DATA>& _frameData)
	{	

		//int numStacks = _inScene->GetSrcObjectCount();
		//for (int i = 0; i < numStacks; ++i)
		//{
		//	KEYFRAME_DATA currKey;
		//	FbxAnimStack* animStack = (FbxAnimStack*)_inScene->GetSrcObject(i);
        //    //animStack->set
		//	// Getting the frame times
		//	FbxTimeSpan animTime = animStack->GetLocalTimeSpan();
		//	currKey.startTime = (float)animTime.GetStart().GetMilliSeconds();
		//	currKey.endTime = (float)animTime.GetStop().GetMilliSeconds();
		//	currKey.durationTime = (float)animTime.GetDuration().GetMilliSeconds();
		//	
		//    // Getting the bones possitions
		//	int numFrames = animStack->GetMemberCount();
		//	for (int iFrame = 0; iFrame < numFrames; ++i)
		//	{
		//		FbxObject *frame = animStack->GetMember(i);
		//		//frame->get
		//	}
		//}



		//bool isAnimated = false;
		//
		//// Iterate all animations (for example, walking, running, falling and etc.)
		//int numAnimations = _inScene->GetSrcObjectCount(FbxAnimStack::ClassId);
		//for (int animationIndex = 0; animationIndex < numAnimations; animationIndex++)
		//{
		//	FbxAnimStack *animStack = (FbxAnimStack*)_inScene->GetSrcObject(FbxAnimStack::ClassId, animationIndex);
		//	FbxAnimEvaluator *animEvaluator = _inScene->GetAnimationEvaluator();
		//	animStack->GetName(); // Get the name of the animation if needed
		//
		//						  // Iterate all the transformation layers of the animation. You can have several layers, for example one for translation, one for rotation, one for scaling and each can have keys at different frame numbers.
		//	int numLayers = animStack->GetMemberCount();
		//	for (int layerIndex = 0; layerIndex < numLayers; layerIndex++)
		//	{
		//		FbxAnimLayer *animLayer = (FbxAnimLayer*)animStack->GetMember(layerIndex);
		//		animLayer->GetName(); // Get the layer's name if needed
		//
		//		FbxAnimCurve *translationCurve = fbxNode->LclTranslation.GetCurve(animLayer);
		//		FbxAnimCurve *rotationCurve = fbxNode->LclRotation.GetCurve(animLayer);
		//		FbxAnimCurve *scalingCurve = fbxNode->LclScaling.GetCurve(animLayer);
		//
		//		if (scalingCurve != 0)
		//		{
		//			int numKeys = scalingCurve->KeyGetCount();
		//			for (int keyIndex = 0; keyIndex < numKeys; keyIndex++)
		//			{
		//				FbxTime frameTime = scalingCurve->KeyGetTime(keyIndex);
		//				FbxDouble3 scalingVector = fbxNode->EvaluateLocalScaling(frameTime);
		//				float x = (float)scalingVector[0];
		//				float y = (float)scalingVector[1];
		//				float z = (float)scalingVector[2];
		//
		//				float frameSeconds = (float)frameTime.GetSecondDouble(); // If needed, get the time of the scaling keyframe, in seconds
		//			}
		//		}
		//		else
		//		{
		//			// If this animation layer has no scaling curve, then use the default one, if needed
		//			FbxDouble3 scalingVector = fbxNode->LclScaling.Get();
		//			float x = (float)scalingVector[0];
		//			float y = (float)scalingVector[1];
		//			float z = (float)scalingVector[2];
		//		}
		//
		//		// Analogically, process rotationa and translation 
		//	}
		//}
	}

} // FBXImporter namespace

void FBXImporter::ExportBinaryFile(const string & _fileName, vector<VERTEX>& _vertecies, vector<unsigned int>& _indices)
{
	//Initializing header for exporting
	ExporterHeader header(FILE_TYPES::MESH, _fileName.c_str());
	header.mesh.numIndex = _indices.size();
	header.mesh.numPoints = _vertecies.size();
	header.mesh.index = INDEX_TYPES::TRI_STRIP;
	header.mesh.vertSize = sizeof(VERTEX);

	fstream binFile;
	binFile.open("FBXBinary.bin", std::ios::out | std::ios::binary);
	if (binFile.is_open())
	{
		binFile.write((char*)&header, sizeof(ExporterHeader));
		for (size_t i = 0; i < _vertecies.size(); i++)
		{
			binFile.write((char*)&_vertecies[i], sizeof(VERTEX));
		}
		for (size_t i = 0; i < _indices.size(); i++)
		{
			binFile.write((char*)&_indices[i], sizeof(unsigned int));
		}
	}
	binFile.close();
}