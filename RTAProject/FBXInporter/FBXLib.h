#pragma once

#include <string>
#include <vector>

using namespace std;
struct VERTEX;
struct TRANSFORM_NODE;

	//int(*GetFunction)(const string & _fileName, vector<VERTEX>& _vertecies,
		//std::vector<int>& _indices, vector<TRANSFORM_NODE>& _transformHierarchy);
#ifdef __cplusplus  

typedef  int(LoadFBXFile(const string & _fileName, vector<VERTEX>& _vertecies, 
	vector<int>& _indices, vector<TRANSFORM_NODE>& _transformHierarchy));

extern "C"
{   
#endif  
	__declspec(dllexport) LoadFBXFile* __cdecl GetFBXLoad(void)
	{
		return 0;
	}
	
#ifdef __cplusplus  
}
#endif  
