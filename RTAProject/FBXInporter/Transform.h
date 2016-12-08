#pragma once
#include <DirectXMath.h>
#include <vector>

__declspec(align(16)) class Transform
{
	Transform* m_parent;
	Transform* m_child;
	Transform* m_sibling;
	const char* m_name;
	bool m_dirty;
public:
	DirectX::XMMATRIX m_localMatrix;
	DirectX::XMMATRIX m_worldMatrix;
	Transform()
	{
		m_child = nullptr;
		m_parent = nullptr;
		m_sibling = nullptr;
	}
	~Transform()
	{
		//if (m_child)
		//	delete m_child;
		//if (m_parent)
		//	delete m_parent;
		//if (m_sibling)
		//	delete m_sibling;
	}
	void AddChild(Transform* _child)
	{
		if (!m_child)
			m_child = _child;
	};
	void AddSibling(Transform* _sibling)
	{
		if (!m_sibling)
			m_sibling = _sibling;
	};
	void AddParent(Transform* _parent)
	{
		if (!m_parent)
			m_parent = _parent;
	};
	void SetName(const char* _name) { m_name = _name; };
	const char* GetName() { return m_name; };
	void SetDirty(bool _dirty) { m_dirty = _dirty; };

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

