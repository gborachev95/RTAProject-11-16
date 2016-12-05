#include "Object.h"
#include "FBXDLL.h"
#include "..//FBXInporter//FBXLib.h"
#include "DDSTextureLoader.h"
#include <fstream>

using namespace FBXImporter;

// Constructor
Object::Object()
{
}

// Destructor
Object::~Object()
{
		delete m_vertecies;
        delete m_indexList;
}

// Instantiates the object using his buffers
void Object::InstantiateModel(ID3D11Device* _device, std::string _filePath, XMFLOAT3 _position, float _shine)
{
	bool result = ReadObject(_filePath, _shine);
	CreateVertexBuffer(_device);
	CreateIndexBuffer(_device);
	CreateConstBuffer(_device);
	m_worldToShader.worldMatrix = XMMatrixIdentity();
	m_worldToShader.worldMatrix.r[3] = { _position.x,_position.y, _position.z,1};
}

// Instantiates the object using an FBX file
void Object::InstantiateFBX(ID3D11Device* _device, std::string _filePath, XMFLOAT3 _position, float _shine)
{
	// Gets us the function from the DLL
	//OpenFBXFile LoadFBXFile = LinkFBXDll();

	// Local variables
	vector<unsigned int> vertexIndices, uvIndices, normalIndices;
	vector<VERTEX> temp_vertices;
	vector<TRANSFORM_NODE> temp_trfNode;
	
	//LoadFBXFile(_filePath, temp_vertices, vertexIndices, temp_trfNode);
	LoadBinaryFile(_filePath, temp_vertices, vertexIndices);

	// Setting the members
	m_numVerts = temp_vertices.size();
	m_vertecies = new VERTEX [m_numVerts];

	// Setting vertecies
	for (unsigned int i = 0; i < m_numVerts; ++i)
		m_vertecies[i] = temp_vertices[i];

	m_numIndicies = vertexIndices.size();
	m_indexList = new unsigned int[m_numIndicies];
	for (unsigned int i = 0; i < m_numIndicies; ++i)
		m_indexList[i] = vertexIndices[i];

	CreateVertexBuffer(_device);
	CreateIndexBuffer(_device);
	CreateConstBuffer(_device);
	m_worldToShader.worldMatrix = XMMatrixIdentity();
	m_worldToShader.worldMatrix.r[3] = { _position.x,_position.y, _position.z,1 };
}

// Draws the object to the screen
void Object::Render(ID3D11DeviceContext* _context, CComPtr<ID3D11ShaderResourceView> _shader)
{
	unsigned int stride = sizeof(VERTEX);
	unsigned int offset = 0;
	
	// Set the buffers for the current object
	_context->IASetVertexBuffers(0, 1, &m_vertexBuffer.p, &stride, &offset);
	_context->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	
	// Set the shader resource - for the texture
	_context->PSSetShaderResources(0, 1,&m_shaderResourceView.p);
	
	// If there is normal mapping, set it as well
	if (_shader.p != nullptr)
		_context->PSSetShaderResources(1, 1, &_shader.p);
	
	// Setting the object const buffer
	_context->VSSetConstantBuffers(0, 1, &m_constBuffer.p);
	_context->PSSetConstantBuffers(0, 1, &m_constBuffer.p);

	// Constant buffer mapping
	D3D11_MAPPED_SUBRESOURCE mapSubresource;
	ZeroMemory(&mapSubresource, sizeof(mapSubresource));
	_context->Map(m_constBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &mapSubresource);
	memcpy(mapSubresource.pData, &m_worldToShader, sizeof(OBJECT_TO_VRAM));
	_context->Unmap(m_constBuffer, NULL);
	

    // Draw the object
	_context->DrawIndexed(m_numIndicies, 0, 0);
}

// Creates the vertex buffer for that object
void Object::CreateVertexBuffer(ID3D11Device* _device)
{
	// Create the vertex buffer storing vertsPoints
	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(bufferDesc));
	bufferDesc.ByteWidth = sizeof(VERTEX) * m_numVerts;
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	bufferDesc.StructureByteStride = sizeof(VERTEX);
	
	// Setting the resource data
	D3D11_SUBRESOURCE_DATA resourceData;
	ZeroMemory(&resourceData, sizeof(resourceData));
	resourceData.pSysMem = m_vertecies;
	_device->CreateBuffer(&bufferDesc, &resourceData, &m_vertexBuffer.p);
}

// Creates the index buffer for that object
void Object::CreateIndexBuffer(ID3D11Device* _device)
{
	// Fill in a buffer description.
	D3D11_BUFFER_DESC bufferDesc;
	ZeroMemory(&bufferDesc, sizeof(D3D11_BUFFER_DESC));
	bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	bufferDesc.ByteWidth = sizeof(unsigned int) * m_numIndicies;
	bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bufferDesc.StructureByteStride = sizeof(unsigned int);

	// Define the resource data.
	D3D11_SUBRESOURCE_DATA initData;
	ZeroMemory(&initData, sizeof(D3D11_SUBRESOURCE_DATA));
	initData.pSysMem = m_indexList;
   
	// Create the buffer with the device.
	_device->CreateBuffer(&bufferDesc, &initData, &m_indexBuffer.p);
}

/*
Reads vertecies, uvs, normals and other object stuff from a .obj file
Followed http://www.opengl-tutorial.org/beginners-tutorials/tutorial-7-model-loading/ tutorial
*/
bool Object::ReadObject(std::string _filePath,float _shine)
{
	// Local variables
	vector<unsigned int> vertexIndices, uvIndices, normalIndices;
	vector<XMFLOAT3> temp_vertices;
	vector<XMFLOAT3> temp_normals;
	vector<XMFLOAT2> temp_uvs;
	FILE *file;
	
	// Opening file - "r" -> read in
	fopen_s(&file, _filePath.c_str(), "r");
	// Check if file opened
	if (file == NULL)
		return false;
	
	// Looping until the file finishes
	while (true)
	{
		// Read the first word of the line
		char lineHeader[128];
		int res = fscanf_s(file, "%s", lineHeader, _countof(lineHeader));
	
		// Check if the file has finished
		if (res == EOF)
			break;
	
		// Check if the line is a vertex
		if (strcmp(lineHeader, "v") == 0)
		{
			XMFLOAT3 vertex;
			fscanf_s(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
			temp_vertices.push_back(vertex);
		}
		// Check if the line is a UV
		else if (strcmp(lineHeader, "vt") == 0)
		{
			XMFLOAT2 uv;
			fscanf_s(file, "%f %f\n", &uv.x, &uv.y);
			temp_uvs.push_back(uv);
		}
		// Check if the line is a normal
		else if (strcmp(lineHeader, "vn") == 0)
		{
			XMFLOAT3 normal;
			fscanf_s(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
			temp_normals.push_back(normal);
		}
		// Check if the line is a index
		else if (strcmp(lineHeader, "f") == 0)
		{
			std::string vertex1, vertex2, vertex3;
			unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
			int matches = fscanf_s(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
			if (matches != 9)
				return false;
	
			// Setting the indicies for the vertecies
			vertexIndices.push_back(vertexIndex[0] - 1);
			vertexIndices.push_back(vertexIndex[1] - 1);
			vertexIndices.push_back(vertexIndex[2] - 1);
	
			// Setting the indicies for the UVs
			uvIndices.push_back(uvIndex[0] - 1);
			uvIndices.push_back(uvIndex[1] - 1);
			uvIndices.push_back(uvIndex[2] - 1);
	
			// Setting the indicies for the normals
			normalIndices.push_back(normalIndex[0] - 1);
			normalIndices.push_back(normalIndex[1] - 1);
			normalIndices.push_back(normalIndex[2] - 1);
		}
	}
	
	// Setting vertecies member
	m_numVerts = vertexIndices.size();
	m_vertecies = new VERTEX[m_numVerts];
	for (unsigned int i = 0; i < m_numVerts; ++i)
	{
		// Setting vertecies
		m_vertecies[i].transform = temp_vertices[vertexIndices[i]];
		// Setting normals
		m_vertecies[i].normals = temp_normals[normalIndices[i]];
		// Setting UVs
		m_vertecies[i].uv.x = temp_uvs[uvIndices[i]].x;
		m_vertecies[i].uv.y = temp_uvs[uvIndices[i]].y;
		m_vertecies[i].uv.z = 0;
		m_vertecies[i].shine.x = _shine;
	}
	
	// Computing the tangents and bitangents
	ComputeTangents();
	// Setting indecies member
	m_numIndicies = vertexIndices.size();
	m_indexList = new unsigned int[m_numIndicies];
	for (unsigned int i = 0; i < m_numIndicies; ++i)
		m_indexList[i] = i;
	
	// Return true if everything went right
	return true;
}

// Textures the object 
void Object::TextureObject(ID3D11Device* _device, const wchar_t* _filePath)
{
	 CreateDDSTextureFromFile(_device, _filePath, NULL, &m_shaderResourceView.p);
}

// Returns the world matrix of the object
XMMATRIX Object::GetWorldMatrix()
{
	return m_worldToShader.worldMatrix;
}

// Sets the world matrix of the object
void Object::SetWorldMatrix(XMMATRIX& _matrix)
{
	m_worldToShader.worldMatrix = _matrix;
}

// Moves the object according to the passed in values
void Object::SetPosition(float _x, float _y, float _z)
{
	m_worldToShader.worldMatrix.r[3].m128_f32[0] = _x;
	m_worldToShader.worldMatrix.r[3].m128_f32[1] = _y;
	m_worldToShader.worldMatrix.r[3].m128_f32[2] = _z;
}

/* 
Used for normal mapping.
Followed http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-13-normal-mapping/ tutorial
Parameters: 
[in] _verts       - vertecies of object
[in] _uvs         - uv coordinates of object
[in] _normals     - normals of object
*/
void Object::ComputeTangents()
{
	//vector<XMFLOAT3> temp_tangents;
	//vector<XMFLOAT3> temp_bitangents;
	//
	//for (unsigned int i = 0; i < m_numVerts; i += 3)
	//{
	//	// Getting the triangles 
	//	XMFLOAT3 tempV0 = m_vertecies[i].transform;
	//	XMFLOAT3 tempV1 = m_vertecies[i + 1].transform;
	//	XMFLOAT3 tempV2 = m_vertecies[i + 2].transform;
	//
	//	XMFLOAT3 tempUV0 = m_vertecies[i].uv;
	//	XMFLOAT3 tempUV1 = m_vertecies[i + 1].uv;
	//	XMFLOAT3 tempUV2 = m_vertecies[i + 2].uv;
	//
	//	// Edges of the triangle 
	//	XMFLOAT3 deltaPos1{ tempV1.x - tempV0.x, tempV1.y - tempV0.y, tempV1.z - tempV0.z };
	//	XMFLOAT3 deltaPos2{ tempV2.x - tempV0.x, tempV2.y - tempV0.y, tempV2.z - tempV0.z };
	//
	//	XMFLOAT3 deltaUV1 = { tempUV1.x - tempUV0.x, tempUV1.y - tempUV0.y, 0 };
	//	XMFLOAT3 deltaUV2 = { tempUV2.x - tempUV0.x, tempUV2.y - tempUV0.y, 0 };
	//
	//	float ratio = (1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x));
	//
	//	// Calculating tangent
	//	XMFLOAT3 tempTangent1 { deltaPos1.x * deltaUV2.y, deltaPos1.y * deltaUV2.y, deltaPos1.z * deltaUV2.y };
	//	XMFLOAT3 tempTangent2{ deltaPos2.x * deltaUV1.y, deltaPos2.y * deltaUV1.y, deltaPos2.z * deltaUV1.y };
	//	XMFLOAT3 tempTangent3{ tempTangent1.x - tempTangent2.x, tempTangent1.y - tempTangent2.y, tempTangent1.z - tempTangent2.z };
	//	XMFLOAT3 tangent{ tempTangent3.x*ratio, tempTangent3.y*ratio, tempTangent3.z*ratio };
	//
	//	// Calculating bitangent
	//	XMFLOAT3 tempBitangent{ tempTangent2.x - tempTangent1.x, tempTangent2.y - tempTangent1.y, tempTangent2.z - tempTangent1.z };
	//	XMFLOAT3 bitangent{ tempBitangent.x*ratio, tempBitangent.y*ratio, tempBitangent.z*ratio };
	//
	//	// Setting them 
	//	m_vertecies[i].tangents = tangent;
	//	m_vertecies[i + 1].tangents = tangent;
	//	m_vertecies[i + 2].tangents = tangent;
	//
	//	m_vertecies[i].bitangents = bitangent;
	//	m_vertecies[i + 1].bitangents = bitangent;
	//	m_vertecies[i + 2].bitangents = bitangent;
	//}
}

void Object::SetShaderResourceView(CComPtr<ID3D11ShaderResourceView> _shader)
{
	m_shaderResourceView = _shader;
}

// Creates constant buffer
void Object::CreateConstBuffer(ID3D11Device* _device)
{
	// Creating the constant buffer for the world
	D3D11_BUFFER_DESC constBufferDesc;
	ZeroMemory(&constBufferDesc, sizeof(D3D11_BUFFER_DESC));
	constBufferDesc.ByteWidth = sizeof(OBJECT_TO_VRAM);
	constBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	constBufferDesc.StructureByteStride = sizeof(float);
	constBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	_device->CreateBuffer(&constBufferDesc, NULL, &m_constBuffer.p);
}

void Object::LoadBinaryFile(std::string _filePath, vector<VERTEX>& _vertecies, vector<unsigned int>& _indices)
{
	FILE* file = nullptr;
	FileInfo::ExporterHeader header;

	if (header.ReadHeader(&file, "FBXBinary.bin", _filePath.c_str()))
	{
		fstream binFile;
		binFile.open("FBXBinary.bin", std::ios::in | std::ios::binary);
		if (binFile.is_open())
		{

		}
		binFile.close();
	}
	
}