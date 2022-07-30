#pragma once
#include "CTexture.h"
class CTexture;

class D3DModel : public AlignedAllocationPolicy<16>
{
private:
	struct VertexType
	{
		XMFLOAT3 position;
		XMFLOAT4 color;
	};

private:
	int				m_vertexCount;
	int				m_indexCount;
	ID3D11Buffer*	m_vertexBuffer;
	ID3D11Buffer*	m_indexBuffer;

	CTexture*		m_pTex;
public:
	void init(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDevCon, const wchar_t* _textureFilePath);
	void render(ID3D11DeviceContext* _pDevCon);
	void Clear();

	int GetIndexCount();

	void LoadTexture(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDevCon, const wchar_t* _strFilePath);
	ID3D11ShaderResourceView* GetTexture() { return m_pTex->GetTexture(); }
	void ReleaseTexture();

private:
	bool InitializeBuffers(ID3D11Device* _pDevice);
	void ShutdownBuffers();
	void RenderBuffers(ID3D11DeviceContext* _pDevCon);

public:
	D3DModel();
	~D3DModel();
};


