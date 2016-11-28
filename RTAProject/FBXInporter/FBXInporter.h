#pragma once  

#include "fbxsdk.h"

#ifdef FBXInporter_EXPORTS  
#define FBXInporter_API __declspec(dllexport)   
#else  
#define FBXInporter_API __declspec(dllimport)   
#endif  


namespace FBXInporter
{
	// This class is exported from the MathLibrary.dll  
	class Functions
	{
	public:
		// Returns a + b  
		static FBXInporter_API int main();
	};
}