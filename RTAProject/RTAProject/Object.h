#pragma once
#include "includes.h"

__declspec(align(16)) class Object
{
	// Graphics variables
	CComPtr<ID3D11Buffer>              m_indexBuffer;
	CComPtr<ID3D11Buffer>              m_vertexBuffer;
	CComPtr<ID3D11Buffer>              m_instanceBuffer;
	CComPtr<ID3D11Texture2D>           m_texture;
	CComPtr<ID3D11ShaderResourceView>  m_shaderResourceView;
	CComPtr<ID3D11Buffer>              m_constBuffer;
	OBJECT_TO_VRAM                     m_worldToShader;       
	// Object variables
	VERTEX                             *m_vertecies;
	unsigned int                       *m_indexList;
	unsigned int                        m_numVerts;
	unsigned int                        m_numIndicies;

	vector<Transform>              m_bones;

	// Private methods that are called only inside of the class
private:
	void CreateVertexBuffer(ID3D11Device* _device);
	void CreateIndexBuffer(ID3D11Device* _device);
	void ComputeTangents();
	bool ReadObject(std::string _filePath, float _shine);
	void CreateConstBuffer(ID3D11Device* _device);
	// Public methods that are called outside of the class
public:

	// Construcor
	Object();
	// Destructor
	~Object();
	void InstantiateModel(ID3D11Device* _device, std::string _filePath, XMFLOAT3 _position, float _shine);
	void InstantiateFBX(ID3D11Device* _device, std::string _filePath, XMFLOAT3 _position, float _shine);
	void Render(ID3D11DeviceContext* _context, CComPtr<ID3D11ShaderResourceView> _shader = nullptr);
	void TextureObject(ID3D11Device* _device, const wchar_t*  _filePath);
	// Getters
	XMMATRIX GetWorldMatrix();

	vector<Transform> GetFBXBones();
	// Setters
	void SetShaderResourceView(CComPtr<ID3D11ShaderResourceView> _shader);
	void SetWorldMatrix(XMMATRIX& _matrix);
	void SetPosition(float _x, float _y, float _z);
	// Loader
	void LoadBinaryFile(std::string _filePath, vector<VERTEX>& _vertecies, vector<unsigned int>& _indices);
	void LoadBinaryFile(std::string _filePath, vector<Transform>& _bones);
	// Alligning by 16 bytes so we don't get a warning 
	void* operator new(size_t i)
	{
		return _mm_malloc(i,16);
	}

		void operator delete(void* p)
	{
		_mm_free(p);
	}
};