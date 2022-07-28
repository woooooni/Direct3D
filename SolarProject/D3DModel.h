#pragma once
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

public:
	void init(ID3D11Device* _pDevice);
	void Clear();
	void render(ID3D11DeviceContext* _pDevCon);

	int GetIndexCount();

private:
	bool InitializeBuffers(ID3D11Device* _pDevice);
	void ShutdownBuffers();
	void RenderBuffers(ID3D11DeviceContext* _pDevCon);

public:
	D3DModel();
	~D3DModel();
};


