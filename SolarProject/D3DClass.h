#pragma once
class D3DClass
{
	SINGLETON(D3DClass);

private:
	bool						m_bVsync;
	int							m_videoCardMemory = 0;
	wstring						m_videoCardDescription;
	IDXGISwapChain*				m_pSwapChain;					// 스왑체인
	ID3D11Device*				m_pDevice;
	ID3D11DeviceContext*		m_pDeviceContext;
	ID3D11RenderTargetView*		m_pRenderTargetView;
	ID3D11Texture2D*			m_pDepthStencilBuffer;
	ID3D11DepthStencilState*	m_pDepthStencilState;
	ID3D11DepthStencilView*		m_pDepthStencilView;
	ID3D11RasterizerState*		m_pRasterizerState;
	XMMATRIX					m_projectionMatrix;
	XMMATRIX					m_worldMatrix;
	XMMATRIX					m_orthoMatrix;

	
public:
	bool Initialize(HWND _hWnd, POINT _ptResolution, bool _bVSync, bool _bFullscreen, float screenDepth, float screenNear);
	ID3D11Device* GetDevice() { return m_pDevice; }
	ID3D11DeviceContext* GetDeviceContext() { return m_pDeviceContext; }


	void SetProjectionMatrix(XMMATRIX _matrix) { m_projectionMatrix = _matrix; };
	void SetWorldMatrix(XMMATRIX _matrix) { m_worldMatrix = _matrix; };
	void SetOrthoMatrix(XMMATRIX _matrix) { m_orthoMatrix = _matrix; };

	XMMATRIX GetProjectionMatrix() { return m_projectionMatrix; };
	XMMATRIX GetWorldMatrix(XMMATRIX _matrix) { return m_worldMatrix; }
	XMMATRIX GetOrthoMatrix(XMMATRIX _matrix) { return m_orthoMatrix; }

	void GetVideoCardInfo(wstring& _cardName, int& _memory) 
	{ 
		_cardName = m_videoCardDescription;
		_memory = m_videoCardMemory;
	}

private:
	void Shutdown();

public:
	void BeginRender(HDC _dc);
	void EndRender(HDC _dc);

};

