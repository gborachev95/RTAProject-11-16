#include "Object.h"
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
	
	ExportObject(_filePath,_shine);
	LoadBinaryFile(_filePath);
	CreateVertexBuffer(_device);
	CreateIndexBuffer(_device);
	CreateConstBuffer(_device);
	m_worldToShader.worldMatrix = XMMatrixIdentity();
	
	m_worldToShader.worldMatrix.r[3] = { _position.x,_position.y, _position.z, 1};
}

// Instantiates the object using an FBX file
void Object::InstantiateFBX(ID3D11Device* _device, std::string _filePath, XMFLOAT3 _position, float _shine)
{
	m_currentFrameIndex = 0;
	// Gets us the function from the DLL
	//OpenFBXFile LoadFBXFile = LinkFBXDll();

	// Local variables
	vector<unsigned int> vertexIndices, uvIndices, normalIndices;
	vector<VERTEX> temp_vertices;


	LoadFBXFile(_filePath, temp_vertices, vertexIndices, m_bones, m_animation);
	temp_vertices.clear();
	vertexIndices.clear();
	m_bones.clear();
	m_animation.SetKeyFramesNumber(0);
	m_animation.SetTotalTime(0);
	m_animation.m_keyFrame.clear();
	LoadBinaryFile(_filePath, temp_vertices, vertexIndices);
	LoadBinaryFile(_filePath, m_bones);
	LoadBinaryFile(_filePath, m_animation);

	// Setting the members
	m_numVerts = temp_vertices.size();
	
	m_vertecies = new VERTEX[m_numVerts];

	// Setting vertecies
	for (unsigned int i = 0; i < m_numVerts; ++i)
	{
		m_vertecies[i] = temp_vertices[i];
		// Setting the shine value
		m_vertecies[i].uv.z = _shine;
	}
	m_numIndicies = vertexIndices.size();
	m_indexList = new unsigned int[m_numIndicies];
	for (unsigned int i = 0; i < m_numIndicies; ++i)
		m_indexList[i] = vertexIndices[i];

	ComputeTangents();
	CreateVertexBuffer(_device);
	CreateIndexBuffer(_device);
	CreateConstBuffer(_device);
	m_worldToShader.worldMatrix = XMMatrixIdentity();
	m_worldToShader.worldMatrix.r[3] = { _position.x,_position.y, _position.z,1 };
}

// Draws the object to the screen
void Object::Render(ID3D11DeviceContext* _context)
{
	unsigned int stride = sizeof(VERTEX);
	unsigned int offset = 0;

	// Set the buffers for the current object
	_context->IASetVertexBuffers(0, 1, &m_vertexBuffer.p, &stride, &offset);
	_context->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);
	_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Set the shader resource - for the texture
	
	_context->PSSetShaderResources(0, 1, &m_defShaderResourceView.p);

	// If there is normal mapping
	if (m_normalShaderResourceView.p != nullptr)
		_context->PSSetShaderResources(1, 1, &m_normalShaderResourceView.p);

	// If there is specular mapping
	if (m_specularShaderResourceView.p != nullptr)
		_context->PSSetShaderResources(2, 1, &m_specularShaderResourceView.p);

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

	// Turn off shaders
	if (m_normalShaderResourceView.p != nullptr)
		_context->PSSetShaderResources(1, 1, &m_defShaderResourceView.p);
	if (m_specularShaderResourceView.p != nullptr)
		_context->PSSetShaderResources(2, 1, &m_defShaderResourceView.p);
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

// Textures the object 
void Object::TextureObject(ID3D11Device* _device, const wchar_t*  _filePathToDefuse, const wchar_t*  _filePathToNormalMap, const wchar_t*  _filePathToSpecular)
{
	
	CreateDDSTextureFromFile(_device, _filePathToDefuse, NULL, &m_defShaderResourceView.p);
	if (_filePathToNormalMap)
		CreateDDSTextureFromFile(_device, _filePathToNormalMap, NULL, &m_normalShaderResourceView.p);
	if (_filePathToSpecular)
		CreateDDSTextureFromFile(_device, _filePathToSpecular, NULL, &m_specularShaderResourceView.p);
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
	vector<XMFLOAT4> temp_tangents;
	vector<XMFLOAT4> temp_bitangents;

	for (unsigned int i = 0; i < m_numVerts; i += 3)
	{
		// Getting the triangles 
		XMFLOAT4 tempV0 = m_vertecies[i].transform;
		XMFLOAT4 tempV1 = m_vertecies[i + 1].transform;
		XMFLOAT4 tempV2 = m_vertecies[i + 2].transform;

		XMFLOAT4 tempUV0 = m_vertecies[i].uv;
		XMFLOAT4 tempUV1 = m_vertecies[i + 1].uv;
		XMFLOAT4 tempUV2 = m_vertecies[i + 2].uv;

		// Edges of the triangle 
		XMFLOAT4 deltaPos1{ tempV1.x - tempV0.x, tempV1.y - tempV0.y, tempV1.z - tempV0.z,0 };
		XMFLOAT4 deltaPos2{ tempV2.x - tempV0.x, tempV2.y - tempV0.y, tempV2.z - tempV0.z,0 };

		XMFLOAT4 deltaUV1 = { tempUV1.x - tempUV0.x, tempUV1.y - tempUV0.y, 0,0 };
		XMFLOAT4 deltaUV2 = { tempUV2.x - tempUV0.x, tempUV2.y - tempUV0.y, 0 ,0 };

		float ratio = (1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x));

		// Calculating tangent
		XMFLOAT4 tempTangent1{ deltaPos1.x * deltaUV2.y, deltaPos1.y * deltaUV2.y, deltaPos1.z * deltaUV2.y,0 };
		XMFLOAT4 tempTangent2{ deltaPos2.x * deltaUV1.y, deltaPos2.y * deltaUV1.y, deltaPos2.z * deltaUV1.y,0 };
		XMFLOAT4 tempTangent3{ tempTangent1.x - tempTangent2.x, tempTangent1.y - tempTangent2.y, tempTangent1.z - tempTangent2.z,0 };
		XMFLOAT4 tangent{ tempTangent3.x*ratio, tempTangent3.y*ratio, tempTangent3.z*ratio,0 };

		// Calculating bitangent
		XMFLOAT4 tempBitangent{ tempTangent2.x - tempTangent1.x, tempTangent2.y - tempTangent1.y, tempTangent2.z - tempTangent1.z,0 };
		XMFLOAT4 bitangent{ tempBitangent.x*ratio, tempBitangent.y*ratio, tempBitangent.z*ratio,0 };

		// Setting them 
		m_vertecies[i].tangents = tangent;
		m_vertecies[i + 1].tangents = tangent;
		m_vertecies[i + 2].tangents = tangent;

		m_vertecies[i].bitangents = bitangent;
		m_vertecies[i + 1].bitangents = bitangent;
		m_vertecies[i + 2].bitangents = bitangent;
	}
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

// Load binary file verticies
void Object::LoadBinaryFile(std::string _filePath, vector<VERTEX>& _vertecies, vector<unsigned int>& _indices)
{
	FILE* file = nullptr;
	FileInfo::ExporterHeader header;
	fstream binFile;

	string binName;
	const char* theName = strrchr(_filePath.c_str(), '\\');
	binName = strrchr(theName, theName[1]);
	binName.pop_back();
	binName.pop_back();
	binName.pop_back();
	binName.pop_back();
	binName.append("_mesh.bin");

	if (header.ReadHeader(&file, binName.c_str(), _filePath.c_str()))
	{
		binFile.open(binName.c_str(), std::ios::in | std::ios::binary);
		if (binFile.is_open())
		{
			binFile.seekp(sizeof(FileInfo::ExporterHeader), ios::beg);
			_vertecies.resize(header.mesh.numPoints);
			binFile.read((char*)&_vertecies[0], (header.mesh.numPoints * header.mesh.vertSize));
			_indices.resize(header.mesh.numIndex);
			binFile.read((char*)&_indices[0], (header.mesh.numIndex * sizeof(unsigned int)));
		}
		binFile.close();
	}
}

// Load binary file bones
void Object::LoadBinaryFile(std::string _filePath, vector<Transform>& _bones)
{
	FILE* file = nullptr;
	FileInfo::ExporterHeader header;
	fstream binFile;
	string binName;

	const char* theName = strrchr(_filePath.c_str(), '\\');
	binName = strrchr(theName, theName[1]);
	binName.pop_back();
	binName.pop_back();
	binName.pop_back();
	binName.pop_back();
	binName.append("_bindpose.bin");

	if (header.ReadHeader(&file, binName.c_str(), _filePath.c_str()))
	{
		binFile.open(binName.c_str(), std::ios::in | std::ios::binary);
		if (binFile.is_open())
		{
			binFile.seekp(sizeof(FileInfo::ExporterHeader), ios::beg);
			_bones.resize(header.bind.numBones);
			binFile.read((char*)&_bones[0], header.bind.numBones * sizeof(Transform));
		}
		binFile.close();
	}
}

// Load binary file animation
void Object::LoadBinaryFile(std::string _filePath, Animation& _animation)
{
	FILE* file = nullptr;
	FileInfo::ExporterHeader header;
	fstream binFile;
	string binName;

	const char* theName = strrchr(_filePath.c_str(), '\\');
	binName = strrchr(theName, theName[1]);
	binName.pop_back();
	binName.pop_back();
	binName.pop_back();
	binName.pop_back();
	binName.append("_anim.bin");

	if (header.ReadHeader(&file, binName.c_str(), _filePath.c_str()))
	{
		binFile.open(binName.c_str(), std::ios::in | std::ios::binary);
		if (binFile.is_open())
		{
			binFile.seekp(sizeof(FileInfo::ExporterHeader), ios::beg);
			binFile.read((char*)&_animation, sizeof(Animation) - sizeof(vector<KeyFrame>));
			_animation.m_keyFrame.resize(header.anim.numFrames);
			for (size_t i = 0; i < header.anim.numFrames; i++)
			{
				binFile.read((char*)&_animation.m_keyFrame[i], sizeof(KeyFrame) - sizeof(vector<Transform>));
				_animation.m_keyFrame[i].m_bones.resize(header.anim.numBones);
				for (size_t j = 0; j < header.anim.numBones; j++)
				{
					binFile.read((char*)&_animation.m_keyFrame[i].m_bones[j], sizeof(Transform));
				}
			}
		}
		binFile.close();
	}
}

// Load Binary File (OBJ)
// Note to Compute Tangents after receiving data
void Object::LoadBinaryFile(std::string _filePath)
{
	fstream binFile;
	string binName;

	string theName = strrchr(_filePath.c_str(), '\\');
	//binName = strrchr(theName.c_str(), theName[1]);
	theName.erase(0,1);
	binName = theName;
	binName.pop_back();
	binName.pop_back();
	binName.pop_back();
	binName.pop_back();
	binName.append("_obj.bin");

	binFile.open(binName.c_str(), std::ios::in | std::ios::binary);
	if (binFile.is_open())
	{
		binFile.read((char*)&m_numVerts, sizeof(unsigned int));
		m_vertecies = new VERTEX[m_numVerts];
		binFile.read((char*)&m_vertecies[0], sizeof(VERTEX) * m_numVerts);
		binFile.read((char*)&m_numIndicies, sizeof(unsigned int));
		m_indexList = new unsigned int[m_numIndicies];
		binFile.read((char*)&m_indexList[0], sizeof(unsigned int) * m_numIndicies);
	}
	binFile.close();

	ComputeTangents();
}

// Getter for the bones
vector<Transform> Object::GetFBXBones()
{
	return m_bones;
}

// Getter for the animation
Animation Object::GetAnimation()
{
	return m_animation;
}

// Adds a frame to the animation
void Object::ForwardFrame()
{
	++m_currentFrameIndex;
	if (unsigned int(m_currentFrameIndex) >= m_animation.GetKeyFramesNumber() - 1)
		m_currentFrameIndex = 1;		
}

// Subtracts a frame form the animation
void Object::BackwardFrame()
{
	--m_currentFrameIndex;
	if (m_currentFrameIndex < 1)//<= 1)
		m_currentFrameIndex = m_animation.GetKeyFramesNumber() - 2;
}

// Returns the current frame index
int Object::GetCurrFrame()
{
	return m_currentFrameIndex;
}

