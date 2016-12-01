#pragma once
#include <string>
#include <vector>

struct VERTEX;
struct TRANSFORM_NODE;

#ifdef FBXImorter_EXPORTS  
#define FBXImporter_API __declspec(dllimport)   
#else  
#define FBXImporter_API __declspec(dllexport) 
#endif  

#if defined(__cplusplus)
extern "C"
namespace FBXImporter
{
	FBXImporter_API int LoadFBXFile(const string & _fileName, vector<VERTEX>& _vertecies, vector<int>& _indices, vector<TRANSFORM_NODE>& _transformHierarchy);
}
#endif
