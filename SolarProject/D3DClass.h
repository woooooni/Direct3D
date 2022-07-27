#pragma once
class D3DClass
{
	SINGLETON(D3DClass);
private:
	bool						m_bVsync;
	int							m_videoCardMemory = 0;
	wchar_t						m_videoCardDescription[128];
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
	bool Initialize(HWND _hWnd, POINT _ptResolution, bool _bVSync, bool _bFullscreen);
	ID3D11Device* GetDevice() { return m_pDevice; }
	ID3D11DeviceContext* GetDeviceContext() { return m_pDeviceContext; }

	void GetProjectionMatrix(XMMATRIX _matrix);
	void GetWorldMatrix(XMMATRIX _matrix);
	void GetOrthoMatrix(XMMATRIX _matrix);
	void GetVideoCardInfo(wchar_t*, int&);


public:
	D3DClass();
	~D3DClass();
};

