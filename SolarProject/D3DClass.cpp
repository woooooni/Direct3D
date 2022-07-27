#include "pch.h"
#include "D3DClass.h"


D3DClass::D3DClass()
	: m_bVsync(false)
	, m_videoCardMemory(0)
	, m_videoCardDescription{0}
	, m_pSwapChain(nullptr)
	, m_pDevice(nullptr)
	, m_pDeviceContext(nullptr)
	, m_pRenderTargetView(nullptr)
	, m_pDepthStencilBuffer(nullptr)
	, m_pDepthStencilState(nullptr)
	, m_pDepthStencilView(nullptr)
	, m_pRasterizerState(nullptr)
	, m_projectionMatrix{}
	, m_worldMatrix{}
	, m_orthoMatrix{}
	
{
}

D3DClass::~D3DClass()
{
}

bool D3DClass::Initialize(HWND _hWnd, POINT _ptResolution, bool _bVSync, bool _bFullscreen, float screenDepth, float screenNear)
{
	
	m_bVsync = _bVSync; // 1. ���� ����ȭ ���� ����.

	// 2. DirectX �׷��� �������̽� ���丮�� ����.
	IDXGIFactory* factory = nullptr;
	if (FAILED(CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory))) return false;

	// 3. ���丮 ��ü�� ����� ù��° �׷��� ī�� �������̽� ����͸� ����.
	IDXGIAdapter* adapter = nullptr;
	if (FAILED(factory->EnumAdapters(0, &adapter))) return false;

	// 4. ���(�����)�� ���� ù��° ����͸� ����.
	IDXGIOutput* adapterOutput = nullptr;
	if (FAILED(adapter->EnumOutputs(0, &adapterOutput))) return false;

	// 5. ��� (�����)�� ���� DXGI_FORMAT_R8G8B8A8_UNORM ǥ�� ���Ŀ� �´� ��� ���� ����.
	unsigned int numModes = 0;
	if (FAILED(adapterOutput->GetDisplayModeList(
													DXGI_FORMAT_R8G8B8A8_UNORM,
													DXGI_ENUM_MODES_INTERLACED,
													&numModes,
													NULL))) 
	{
		return false;
	}

	// 6. ������ ��� ����Ϳ� �׷���ī�� ������ ������ ����Ʈ�� ����.
	DXGI_MODE_DESC* displayModeList = new DXGI_MODE_DESC[numModes];
	if (!displayModeList) return false;

	// 7. ���÷��� ��忡 ���� ����Ʈ�� ä��ϴ�
	if (FAILED(adapterOutput->GetDisplayModeList(
													DXGI_FORMAT_R8G8B8A8_UNORM,
													DXGI_ENUM_MODES_INTERLACED,
													&numModes, displayModeList)))
	{
		return false;
	}

	//8. ��� ���÷��� ��忡 ���� ȭ���� �ʺ�/���̿� �´� ���÷��� ��带 ã��.
	//   (���� ������ ���� ã����, ������� ���ΰ�ħ ������ �и�� ���ڰ��� ������.)
	UINT numerator = 0;
	UINT denominator = 0;

	for (UINT i = 0; i < numModes; i++) 
	{
		if (displayModeList[i].Width == _ptResolution.x && 
			displayModeList[i].Height == _ptResolution.y)
		{
			numerator = displayModeList[i].RefreshRate.Numerator;
			denominator = displayModeList[i].RefreshRate.Denominator;
		}
	}
	
	// 9. ���� ī�� ����ü ���
	DXGI_ADAPTER_DESC adapterDesc;
	if (FAILED(adapter->GetDesc(&adapterDesc))) return false;
	
	// 10. ����ī�� �޸� �뷮 ������ �ް�����Ʈ ������ ����.
	m_videoCardMemory = (int)(adapterDesc.DedicatedVideoMemory / 1024 / 1024);

	// ����ī�� �̸� ����.
	// TODO:: ��ü�� �Լ� Ȥ�� ���� ����.
	/*size_t stringLength = 0;
	if (wcstombs_s(&stringLength, m_videoCardDescription, 128, strName, 128) != 0) return false;*/

	m_videoCardDescription = adapterDesc.Description;

	// 11. ����� �� ���丮 ���� ���� ����
	// ���÷��� ��� ����Ʈ�� ����
	delete[] displayModeList;
	displayModeList = 0;

	// ��� ��͸� ����
	adapterOutput->Release();
	adapterOutput = 0;

	// ��͸� ����
	adapter->Release();
	adapter = 0;

	// ���丮 ��ü�� ����
	factory->Release();
	factory = 0;

	// 12. ����ü�� ����ü �ʱ�ȭ
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	ZeroMemory(&swapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));

	// ����� ���� ����.
	swapChainDesc.BufferCount = 1;

	//����� ����, ���� ����
	swapChainDesc.BufferDesc.Width = _ptResolution.x;
	swapChainDesc.BufferDesc.Height = _ptResolution.y;

	//32bit �����̽� ����.
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	//����� ���ΰ�ħ ���� ����(��������ȭ ó��)
	if (m_bVsync)
	{
		swapChainDesc.BufferDesc.RefreshRate.Numerator = numerator;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = denominator;
	}
	else
	{
		swapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
		swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	}

	//����� ���뵵 ����.
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

	//�������� ���� ������ �ڵ� ����.
	swapChainDesc.OutputWindow = _hWnd;


	//��Ƽ���ø� ����
	swapChainDesc.SampleDesc.Count = 1;		//�ȼ��� ������ ǥ�� ����
	swapChainDesc.SampleDesc.Quality = 0;	// ���ϴ� ǰ�� ����

	//â��� or Ǯ��ũ�� ����
	if (_bFullscreen)
	{
		swapChainDesc.Windowed = false;
	}
	else
	{
		swapChainDesc.Windowed = true;
	}

	// ��ĵ���� ���� �� ũ�⸦ �������� �������� ����.
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	// ��µ� ���� ����۸� ��쵵�� ����.
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	// �߰� �ɼ� �÷��׸� ������� ����.
	swapChainDesc.Flags = 0;

	//��ó���� DirectX 11�� ����.
	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;

	// 13. ����ü��, Direct3D ��ġ �� Direct3D ��ġ ���ؽ�Ʈ ����
	if (FAILED(D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, &featureLevel, 1,
		D3D11_SDK_VERSION, &swapChainDesc, &m_pSwapChain, &m_pDevice, NULL, &m_pDeviceContext)))
	{
		return false;
	}

	// 14. RenderTargetView �ʱ�ȭ
	// ����� �����͸� ������
	ID3D11Texture2D* backBufferPtr = nullptr;
	if (FAILED(m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBufferPtr))) return false;

	// �� ���� �����ͷ� ���� Ÿ�� �� ����
	if (FAILED(m_pDevice->CreateRenderTargetView(backBufferPtr, NULL, &m_pRenderTargetView))) return false;

	// ����� ������ ����
	backBufferPtr->Release();
	backBufferPtr = 0;


	// 15. ���� ���� �ؽ��� ����
	// ���� ���� ����ü�� �ʱ�ȭ
	D3D11_TEXTURE2D_DESC depthBufferDesc;
	ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));

	// ���� ���� ����ü�� �ۼ�
	depthBufferDesc.Width = _ptResolution.x;
	depthBufferDesc.Height = _ptResolution.y;
	depthBufferDesc.MipLevels = 1;
	depthBufferDesc.ArraySize = 1;
	depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthBufferDesc.SampleDesc.Count = 1;
	depthBufferDesc.SampleDesc.Quality = 0;
	depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthBufferDesc.CPUAccessFlags = 0;
	depthBufferDesc.MiscFlags = 0;

	// ������ ���̹��� ����ü�� ����Ͽ� ���� ���� �ؽ��ĸ� ����
	if (FAILED(m_pDevice->CreateTexture2D(&depthBufferDesc, NULL, &m_pDepthStencilBuffer))) return false;


	// 16. ���ٽ� ���� ���� ����.
	// ���ٽ� ���� ����ü�� �ʱ�ȭ
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
	ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));

	// ���ٽ� ���� ����ü�� �ۼ�
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

	depthStencilDesc.StencilEnable = true;
	depthStencilDesc.StencilReadMask = 0xFF;
	depthStencilDesc.StencilWriteMask = 0xFF;

	// �ȼ� ������ ���ٽ� ����
	depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// �ȼ� �޸��� ���ٽ� ����
	depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// ���� ���ٽ� ���¸� ����
	if (FAILED(m_pDevice->CreateDepthStencilState(&depthStencilDesc, &m_pDepthStencilState))) return false;


	// 17. ���� ���ٽ� ���� ����.
	m_pDeviceContext->OMSetDepthStencilState(m_pDepthStencilState, 1);

	// 18. ����/���ٽ� �� ���ε�
	// ���� ���ٽ� ���� ����ü�� �ʱ�ȭ
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));

	// ���� ���ٽ� �� ����ü�� ����
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	// ���� ���ٽ� �並 �����մϴ�
	if (FAILED(m_pDevice->CreateDepthStencilView(m_pDepthStencilBuffer, &depthStencilViewDesc, &m_pDepthStencilView)))
	{
		return false;
	}

	// ������ ��� ��� ���� ���ٽ� ���۸� ��� ���� ������ ���ο� ���ε�
	m_pDeviceContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);


	// 19. ������ ������ ����
	// �׷����� ������� ����� ������ ������ ����ü�� ����
	D3D11_RASTERIZER_DESC rasterDesc;
	rasterDesc.AntialiasedLineEnable = false;
	rasterDesc.CullMode = D3D11_CULL_BACK;
	rasterDesc.DepthBias = 0;
	rasterDesc.DepthBiasClamp = 0.0f;
	rasterDesc.DepthClipEnable = true;
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.FrontCounterClockwise = false;
	rasterDesc.MultisampleEnable = false;
	rasterDesc.ScissorEnable = false;
	rasterDesc.SlopeScaledDepthBias = 0.0f;

	// ��� �ۼ��� ����ü���� ������ ������ ���¸����
	if (FAILED(m_pDevice->CreateRasterizerState(&rasterDesc, &m_pRasterizerState)))
	{
		return false;
	}

	// ������ ������ ���¸� ����
	m_pDeviceContext->RSSetState(m_pRasterizerState);


	//20. ����Ʈ ����
	// �������� ���� ����Ʈ�� ����
	D3D11_VIEWPORT viewport;
	viewport.Width = (float)_ptResolution.x;
	viewport.Height = (float)_ptResolution.y;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;

	// ����Ʈ ����
	m_pDeviceContext->RSSetViewports(1, &viewport);


	// 21. ���� ��� ���� (3������ ��ü�� 2�������� ǥ���ϱ� ���� ���� ���.) 
	//-> ���������� ���ľ� 2D����Ϳ� ����� �����ϱ� ������ �ʼ� ������.
	// ���� ��� ����
	// FOV(Field Of View) -> ȭ�鿡 ǥ���� �þ��� ������ ǥ���ϴ� ��. �����ִ� ���� ��ŭ ������ų ȭ���� ������ ������.
	float fieldOfView = XM_PI / 4.0f;
	float screenAspect = (float)_ptResolution.x / (float)_ptResolution.y;


	// 3D ������������ ���� ��� �����
	// TODO:: ����.
	m_projectionMatrix = XMMatrixPerspectiveFovLH(fieldOfView, screenAspect, screenNear, screenDepth);


	// 22. ���� ��� ����
	// ���� ����� �𵨸��� ���� ��ǥ�� �Ű��ٶ� ����ϴ� �����.
	// TODO:: ����
	// ���� ����� �׵� ��ķ� �ʱ�ȭ�մϴ�
	m_worldMatrix = XMMatrixIdentity();


	// 23. ���� ���� ���
	// UIó�� ���ٰ��� �ʿ���� ��ü�� �Ϲ� ��������� �ƴ� ���� ��������� �ʿ���. -> FOV�� ���� ��.
	// 2D ������������ ���� ���� ����� ����ϴ�
	// TODO:: ����.
	m_orthoMatrix = XMMatrixOrthographicLH((float)_ptResolution.x, (float)_ptResolution.y, screenNear, screenDepth);

	return true;
}


void D3DClass::BeginRender(HDC _dc)
{
	// ���۸� ���� ���� �����մϴ�
	float color[4] = { .5f, .5f, .5f, 1.f };

	// ����۸� ����ϴ�
	m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView, color);

	// ���� ���۸� ����ϴ�
	m_pDeviceContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
}



void D3DClass::EndRender(HDC _dc)
{
	// �������� �Ϸ�Ǿ����Ƿ� ȭ�鿡 �� ���۸� ǥ���մϴ�.
	if (m_bVsync)
	{
		// ȭ�� ���� ��ħ ������ �����մϴ�.
		m_pSwapChain->Present(1, 0);
	}
	else
	{
		// ������ ������ ���
		m_pSwapChain->Present(0, 0);
	}
}

void D3DClass::Shutdown()
{
	// ���� �� ������ ���� �������� ������ ���� ü���� ���� �� �� ���ܰ� �߻��մϴ�.
	if (m_pSwapChain)
	{
		m_pSwapChain->SetFullscreenState(false, NULL);
	}

	if (m_pRasterizerState)
	{
		m_pRasterizerState->Release();
		m_pRasterizerState = 0;
	}

	if (m_pDepthStencilView)
	{
		m_pDepthStencilView->Release();
		m_pDepthStencilView = 0;
	}

	if (m_pDepthStencilState)
	{
		m_pDepthStencilState->Release();
		m_pDepthStencilState = 0;
	}

	if (m_pDepthStencilBuffer)
	{
		m_pDepthStencilBuffer->Release();
		m_pDepthStencilBuffer = 0;
	}

	if (m_pRenderTargetView)
	{
		m_pRenderTargetView->Release();
		m_pRenderTargetView = 0;
	}

	if (m_pDeviceContext)
	{
		m_pDeviceContext->Release();
		m_pDeviceContext = 0;
	}

	if (m_pDevice)
	{
		m_pDevice->Release();
		m_pDevice = 0;
	}

	if (m_pSwapChain)
	{
		m_pSwapChain->Release();
		m_pSwapChain = 0;
	}
}


