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

		vector<ANIMATION_DATA> temp;
		GetAnimationData(fbxScene, temp);

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
		vector<FbxVector2> UVList;
		vector<FbxVector4> normalsList;
		vector<VERTEX> controlPointsList;
		vector<VERTEX> vertexList;

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
			controlPointsList.push_back(currVertex);
		}

		// Loop for each poly
		for (int Poly =0; Poly < currMesh->GetPolygonCount(); Poly++)
		{
			// Get number of verts in this poly
			const int NumVertices = currMesh->GetPolygonSize(Poly);

			// Loop for each vert in poly
			for (int Vertex = 0; Vertex < NumVertices; Vertex++)
			{
				FbxVector2 fbxTexCoord;
				FbxStringList UVSetNameList;
				FbxVector4 fbxNormals;
				VERTEX pos;

				// Get the name of each set of UV coords
				currMesh->GetUVSetNames(UVSetNameList);

				//Get Data
				bool map;
				currMesh->GetPolygonVertexUV(Poly, Vertex, UVSetNameList.GetStringAt(0), fbxTexCoord, map);
				currMesh->GetPolygonVertexNormal(Poly, Vertex, fbxNormals);
				pos = controlPointsList[_indicies[Poly * 3 + Vertex]];

				pos.normals.x = fbxNormals.mData[0];
				pos.normals.y = fbxNormals.mData[1];
				pos.normals.z = fbxNormals.mData[2];
				pos.uv.x = fbxTexCoord[0];
				pos.uv.y = 1- fbxTexCoord[1];

				FbxVector2 UVCoord;
				UVCoord.mData[0] = static_cast<float>(fbxTexCoord[0]);
				UVCoord.mData[1] = static_cast<float>(fbxTexCoord[1]);
				
				// Store Data
				UVList.push_back(UVCoord);
				normalsList.push_back(fbxNormals);
				vertexList.push_back(pos);
				_vertecies.push_back(pos);
				_indicies[Poly * 3 + Vertex] = Poly * 3 + Vertex;
			}
		}

		FbxVector4 cpoint = currMesh->GetControlPointAt(82);

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

		//int vertexCounter = 0;
		//fbxsdk::FbxGeometryElementNormal* normalEl = currMesh->GetElementNormal();
		//int polygonCount = currMesh->GetPolygonCount();
		//for (int i = 0; i < polygonCount; ++i) 
		//{
		//	FbxLayerElementArrayTemplate<fbxsdk::FbxVector2>* uvVertices = 0;
		//	currMesh->GetTextureUV(&uvVertices, fbxsdk::FbxLayerElement::eTextureDiffuse);
		//	for (int j = 0; j < currMesh->GetPolygonSize(i); ++j) 
		//	{
		//		// Setting Vertecies
		//		VERTEX currVertex;
		//		currVertex.transform = XMFLOAT3(float(currMesh->GetControlPointAt(vertexCounter).mData[0]), float(currMesh->GetControlPointAt(vertexCounter).mData[1]), float(currMesh->GetControlPointAt(vertexCounter).mData[2]));

		//		// Setting Normals
		//		FbxVector4 normal = normalEl->GetDirectArray().GetAt(vertexCounter);
		//		currVertex.normals.x = (float)normal[0];
		//		currVertex.normals.y = (float)normal[1];
		//		currVertex.normals.z = (float)normal[2];
		//		

		//		// Setting UVs
		//		currVertex.uv.x = (float)(*uvVertices)[vertexCounter].mData[0];
		//		currVertex.uv.y = (float)(*uvVertices)[vertexCounter].mData[1];

		//		++vertexCounter;

		//		// Pushing the final result
		//		_vertecies.push_back(currVertex);
		//	}

		//}

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
			//bone->GetParent();
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

	void GetFrameData(FbxNode* _inNode, std::vector<KEYFRAME_DATA>& _frameData)
	{	
	}

	void GetAnimationData(FbxScene* _inScene, std::vector<ANIMATION_DATA> _animData)
	{
		int numStacks = _inScene->GetSrcObjectCount();
		for (int i = 0; i < numStacks; ++i)
		{
			FbxAnimStack* animStack = (FbxAnimStack*)_inScene->GetSrcObject(i);
			int numAnimLayers = animStack->GetMemberCount();

			for (int layerIndex = 0; layerIndex < numAnimLayers; ++i)
			{
				//FbxAnimLayer* lAnimLayer = animStack->GetMember(i);
			}
		}
	}

#if 0
	//void ProcessSkeletonHierarchy(FbxNode* inRootNode)
	//{
	//
	//	for (int childIndex = 0; childIndex < inRootNode->GetChildCount(); ++childIndex)
	//	{
	//		FbxNode* currNode = inRootNode->GetChild(childIndex);
	//		ProcessSkeletonHierarchyRecursively(currNode, 0, 0, -1);
	//	}
	//}

	// inDepth is not needed here, I used it for debug but forgot to remove it
	//void ProcessSkeletonHierarchyRecursively(FbxNode* inNode, int inDepth, int myIndex, int inParentIndex)
	//{
	//	if (inNode->GetNodeAttribute() && inNode->GetNodeAttribute()->GetAttributeType() && inNode->GetNodeAttribute()->GetAttributeType() == FbxNodeAttribute::eSkeleton)
	//	{
	//		//Joint currJoint;
	//		//currJoint.mParentIndex = inParentIndex;
	//		//currJoint.mName = inNode->GetName();
	//		//mSkeleton.mJoints.push_back(currJoint);
	//	}
	//	for (int i = 0; i < inNode->GetChildCount(); i++)
	//	{
	//		//ProcessSkeletonHierarchyRecursively(inNode->GetChild(i), inDepth + 1, mSkeleton.mJoints.size(), myIndex);
	//	}
	//}

	//void ProcessJointsAndAnimations(FbxNode* inNode)
	//{
	//	//FbxMesh* currMesh = inNode->GetMesh();
	//	//unsigned int numOfDeformers = currMesh->GetDeformerCount();
	//	// This geometry transform is something I cannot understand
	//	// I think it is from MotionBuilder
	//	// If you are using Maya for your models, 99% this is just an
	//	// identity matrix
	//	// But I am taking it into account anyways......
	//	//FbxAMatrix geometryTransform = Utilities::GetGeometryTransformation(inNode);
	//
	//	// A deformer is a FBX thing, which contains some clusters
	//	// A cluster contains a link, which is basically a joint
	//	// Normally, there is only one deformer in a mesh
	//	//for (unsigned int deformerIndex = 0; deformerIndex < numOfDeformers; ++deformerIndex)
	//	//{
	//	//	// There are many types of deformers in Maya,
	//	//	// We are using only skins, so we see if this is a skin
	//	//	FbxSkin* currSkin = reinterpret_cast<FbxSkin*>(currMesh->GetDeformer(deformerIndex, FbxDeformer::eSkin));
	//	//	if (!currSkin)
	//	//	{
	//	//		continue;
	//	//	}
	//	//
	//	//	unsigned int numOfClusters = currSkin->GetClusterCount();
	//	//	for (unsigned int clusterIndex = 0; clusterIndex < numOfClusters; ++clusterIndex)
	//	//	{
	//	//		FbxCluster* currCluster = currSkin->GetCluster(clusterIndex);
	//	//		std::string currJointName = currCluster->GetLink()->GetName();
	//	//		unsigned int currJointIndex = FindJointIndexUsingName(currJointName);
	//	//		FbxAMatrix transformMatrix;
	//	//		FbxAMatrix transformLinkMatrix;
	//	//		FbxAMatrix globalBindposeInverseMatrix;
	//	//
	//	//		currCluster->GetTransformMatrix(transformMatrix);	// The transformation of the mesh at binding time
	//	//		currCluster->GetTransformLinkMatrix(transformLinkMatrix);	// The transformation of the cluster(joint) at binding time from joint space to world space
	//	//		globalBindposeInverseMatrix = transformLinkMatrix.Inverse() * transformMatrix * geometryTransform;
	//	//
	//	//		// Update the information in mSkeleton 
	//	//		mSkeleton.mJoints[currJointIndex].mGlobalBindposeInverse = globalBindposeInverseMatrix;
	//	//		mSkeleton.mJoints[currJointIndex].mNode = currCluster->GetLink();
	//	//
	//	//		// Associate each joint with the control points it affects
	//	//		unsigned int numOfIndices = currCluster->GetControlPointIndicesCount();
	//	//		for (unsigned int i = 0; i < numOfIndices; ++i)
	//	//		{
	//	//			BlendingIndexWeightPair currBlendingIndexWeightPair;
	//	//			currBlendingIndexWeightPair.mBlendingIndex = currJointIndex;
	//	//			currBlendingIndexWeightPair.mBlendingWeight = currCluster->GetControlPointWeights()[i];
	//	//			mControlPoints[currCluster->GetControlPointIndices()[i]]->mBlendingInfo.push_back(currBlendingIndexWeightPair);
	//	//		}
	//	//
	//	//		// Get animation information
	//	//		// Now only supports one take
	//	//		FbxAnimStack* currAnimStack = mFBXScene->GetSrcObject<FbxAnimStack>(0);
	//	//		FbxString animStackName = currAnimStack->GetName();
	//	//		mAnimationName = animStackName.Buffer();
	//	//		FbxTakeInfo* takeInfo = mFBXScene->GetTakeInfo(animStackName);
	//	//		FbxTime start = takeInfo->mLocalTimeSpan.GetStart();
	//	//		FbxTime end = takeInfo->mLocalTimeSpan.GetStop();
	//	//		mAnimationLength = end.GetFrameCount(FbxTime::eFrames24) - start.GetFrameCount(FbxTime::eFrames24) + 1;
	//	//		Keyframe** currAnim = &mSkeleton.mJoints[currJointIndex].mAnimation;
	//	//
	//	//		for (FbxLongLong i = start.GetFrameCount(FbxTime::eFrames24); i <= end.GetFrameCount(FbxTime::eFrames24); ++i)
	//	//		{
	//	//			FbxTime currTime;
	//	//			currTime.SetFrame(i, FbxTime::eFrames24);
	//	//			*currAnim = new Keyframe();
	//	//			(*currAnim)->mFrameNum = i;
	//	//			FbxAMatrix currentTransformOffset = inNode->EvaluateGlobalTransform(currTime) * geometryTransform;
	//	//			(*currAnim)->mGlobalTransform = currentTransformOffset.Inverse() * currCluster->GetLink()->EvaluateGlobalTransform(currTime);
	//	//			currAnim = &((*currAnim)->mNext);
	//	//		}
	//	//	}
	//	//}
	//	//
	//	//// Some of the control points only have less than 4 joints
	//	//// affecting them.
	//	//// For a normal renderer, there are usually 4 joints
	//	//// I am adding more dummy joints if there isn't enough
	//	//BlendingIndexWeightPair currBlendingIndexWeightPair;
	//	//currBlendingIndexWeightPair.mBlendingIndex = 0;
	//	//currBlendingIndexWeightPair.mBlendingWeight = 0;
	//	//for (auto itr = mControlPoints.begin(); itr != mControlPoints.end(); ++itr)
	//	//{
	//	//	for (unsigned int i = itr->second->mBlendingInfo.size(); i <= 4; ++i)
	//	//	{
	//	//		itr->second->mBlendingInfo.push_back(currBlendingIndexWeightPair);
	//	//	}
	//	//}
	//}

#endif 

} // FBXImporter namespace