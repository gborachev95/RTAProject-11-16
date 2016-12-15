#pragma once

//#include "FBXInporter.h"

// Standart Includes
#include <iostream>
#include <Windows.h>
#include <time.h>
#include <vector>
#include <fstream>
#include <string>
#include <DirectXMath.h>
#include <d3d11.h> 
// For smart pointers
#include <atlbase.h>

// Application includes
#include "..\FBXInporter\ExporterHeader.h"
#include "Transform.h"
#include "Animation.h"

// Defines
#define WINDOW_HEIGHT 700
#define WINDOW_WIDTH 700
const float SCREEN_ZFAR = 100.0f;
const float SCREEN_ZNEAR = 0.01f;

// Namespaces
using namespace DirectX;
using namespace std;

// Loading libraries
#pragma comment (lib, "d3d11.lib") 

// Structures
struct VERTEX
{
	XMFLOAT4 transform;
	XMFLOAT4 normals;
	XMFLOAT4 uv;
	XMFLOAT4 tangents;
	XMFLOAT4 bitangents;
	XMFLOAT4 skinIndices;
	XMFLOAT4 skinWeights;
};

struct COLOR
{
	union {
		struct
		{
			float r;
			float g;
			float b;
			float a;
		};

		float color[4];
	};

	float* GetColor()
	{
		return color;
	}
};

struct OBJECT_TO_VRAM
{
	XMMATRIX worldMatrix;
};

struct SCENE_TO_VRAM
{
	XMMATRIX viewMatrix;
	XMMATRIX projectionMatrix;
	XMFLOAT3 cameraPosition;
};

struct BONES_TO_VRAM
{
	
	XMMATRIX bones[38];
	XMMATRIX positionOffset;
};

struct LIGHT_TO_VRAM
{
	XMVECTOR transform;
	XMFLOAT3 direction;
	float    radius;
	COLOR    color;
	bool     status;
	bool     padding[3];
};


