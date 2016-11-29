#pragma once
#include "includes.h"

class Object
{
	// Graphics variables
	CComPtr<ID3D11Buffer>              m_indexBuffer;
	CComPtr<ID3D11Buffer>              m_vertexBuffer;
	CComPtr<ID3D11Buffer>              m_instanceBuffer;
	CComPtr<ID3D11Texture2D>           m_texture;
	CComPtr<ID3D11ShaderResourceView>  m_shaderResourceView;
	OBJECT_TO_VRAM                     m_worldToShader;
	// Object variables
	VERTEX               *m_vertecies;
	unsigned int         *m_indexList;
	unsigned int          m_numVerts;
	unsigned int          m_numIndicies;

	// Private methods that are called only inside of the class
private:
	void CreateVertexBuffer(ID3D11Device* _device);
	void CreateIndexBuffer(ID3D11Device* _device);
	void ComputeTangents();
	bool ReadObject(std::string _filePath, float _shine);

	// Public methods that are called outside of the class
public:

	// Construcor
	Object();
	// Destructor
	~Object();
	void InstantiateModel(ID3D11Device* _device, std::string _filePath, XMFLOAT3 _position, float _shine);
	void InstantiateFBX(ID3D11Device* _device, std::string _filePath, XMFLOAT3 _position, float _shine);
	void Render(ID3D11DeviceContext* _context, ID3D11Buffer* _constBuffer, CComPtr<ID3D11ShaderResourceView> _shader = nullptr);
	void TextureObject(ID3D11Device* _device, const wchar_t*  _filePath);
	// Getters
	XMMATRIX GetWorldMatrix();
	// Setters
	void SetShaderResourceView(CComPtr<ID3D11ShaderResourceView> _shader);
	void SetWorldMatrix(XMMATRIX& _matrix);
	void SetPosition(float _x, float _y, float _z);

};