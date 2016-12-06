#pragma once
#include <Windows.h>
#include <string>
#include <vector>
struct VERTEX;
class Transform;

using namespace std;


typedef int (WINAPI* OpenFBXFile)(const string & _fileName, vector<VERTEX>& _vertecies, vector<unsigned int>& _indices, vector<Transform>& _transformHierarchy);

OpenFBXFile LinkFBXDll()
{
	HINSTANCE dllInstance = LoadLibrary(L"FbxInporter.dll");

	if (dllInstance != NULL)
	{
		OpenFBXFile fbxFunction = (OpenFBXFile)GetProcAddress(dllInstance, "LoadFBXFile");
		if (fbxFunction != NULL)
			return fbxFunction;
	}

	return NULL;
}