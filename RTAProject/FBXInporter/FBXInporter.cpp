// FBXInporter.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"  
#include "FBXInporter.h"  

namespace FBXInporter
{
	int Functions::main()
	{

		// Change the following filename to a suitable filename value.
		const char* lFilename = "file.fbx";

		// Initialize the SDK manager. This object handles memory management.
		FbxManager* lSdkManager = FbxManager::Create();

		//To import the contents of an FBX file, a FbxIOSettings object and a FbxImporter object must be created.
		//A FbxImporter object is initialized by providing the filename of the file to import along with a 
		//FbxIOSettings object that has been appropriately configured to suit the importing needs(see I / O Settings).

		// Create the IO settings object.
		FbxIOSettings *ios = FbxIOSettings::Create(lSdkManager, IOSROOT);
		lSdkManager->SetIOSettings(ios);

		// Create an importer using the SDK manager.
		FbxImporter* lImporter = FbxImporter::Create(lSdkManager, "");

		// Use the first argument as the filename for the importer.
		if (!lImporter->Initialize(lFilename, -1, lSdkManager->GetIOSettings())) {
			printf("Call to FbxImporter::Initialize() failed.\n");
			printf("Error returned: %s\n\n", lImporter->GetStatus().GetErrorString());
			exit(-1);
		}

		//The FbxImporter object populates a provided FbxScene object with the elements contained in the FBX file.
		//Observe that an empty string is passed as the second parameter in the FbxScene::Create() function.
		//Objects created in the FBX SDK can be given arbitrary, non - unique names, that allow the user 
		//or other programs to identify the object after it is exported.After the FbxScene is populated,
		//the FbxImporter can be safely destroyed.
			
		// Create a new scene so that it can be populated by the imported file.
		FbxScene* lScene = FbxScene::Create(lSdkManager, "myScene");

		// Import the contents of the file into the scene.
		lImporter->Import(lScene);

		// The file is imported, so get rid of the importer.
		lImporter->Destroy();

		return 0;
	}
}