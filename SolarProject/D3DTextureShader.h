#pragma once
class D3DTextureShader : public AlignedAllocationPolicy<16>
{
private:
	struct MatrixBufferType
	{
		XMMATRIX world;
		XMMATRIX view;
		XMMATRIX projection;
	};

private:
	ID3D11VertexShader*		m_vertexShader;
	ID3D11PixelShader*		m_pixelShader;
	ID3D11InputLayout*		m_layout;
	ID3D11Buffer*			m_matrixBuffer;

	ID3D11SamplerState*		m_sampleState;

public:
	void init(ID3D11Device* _pDevice, HWND _hWnd);
	void render(ID3D11DeviceContext* _pDevCon, int _index, XMMATRIX _worldMat, XMMATRIX _viewMat, XMMATRIX _projMat, ID3D11ShaderResourceView* _pResource);
	void Clear();

private:
	bool InitializeShader(ID3D11Device* _pDevice, HWND _hWnd, const wchar_t* _vsFileName, const wchar_t* _psFileName);
	void ShutDownShader();
	void OutputShaderErrorMessage(ID3D10Blob* _pErr, HWND _hWnd, const wchar_t* _shaderFileName);

	bool SetShaderParameters(ID3D11DeviceContext* _pDevCon, XMMATRIX _worldMat, XMMATRIX _viewMat, XMMATRIX _projMat, ID3D11ShaderResourceView* _pResource);
	void RenderShader(ID3D11DeviceContext* _pDevCon, int _index);


public:
	D3DTextureShader();
	~D3DTextureShader();
};

