#pragma once
#include <string>
#include <vector>

struct VERTEX;
class Transform;
class Animation;

#ifdef FBXImorter_EXPORTS  
#define FBXImporter_API __declspec(dllimport)   
#else  
#define FBXImporter_API __declspec(dllexport) 
#endif  

#if defined(__cplusplus)
extern "C"
namespace FBXImporter
{
	FBXImporter_API int LoadFBXFile(const string & _fileName, vector<VERTEX>& _vertecies, vector<unsigned int>& _indices, vector<Transform>& _transformHierarchy, Animation& _animation);
	FBXImporter_API bool ExportObject(string filePath);
}
#endif
