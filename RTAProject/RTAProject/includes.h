#pragma once
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

#include "DDSTextureLoader.h"
// Application includes
#include  "OBJECT_VS.csh"
#include  "OBJECT_PS.csh"

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

// Globals

// Structures
struct VERTEX
{
	XMFLOAT3 transform;
	XMFLOAT3 normals;
	XMFLOAT3 uv;
	XMFLOAT3 tangents;
	XMFLOAT3 bitangents;
	XMFLOAT3 shine;
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

struct LIGHT_TO_VRAM
{
	XMVECTOR transform;
	XMFLOAT3 direction;
	float    radius;
	COLOR    color;
	bool     status;
	bool     padding[3];
};
