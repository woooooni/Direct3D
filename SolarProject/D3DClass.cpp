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
	
	m_bVsync = _bVSync; // 1. 수직 동기화 상태 저장.

	// 2. DirectX 그래픽 인터페이스 팩토리를 생성.
	IDXGIFactory* factory = nullptr;
	if (FAILED(CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&factory))) return false;

	// 3. 팩토리 객체를 사용해 첫번째 그래픽 카드 인터페이스 어댑터를 생성.
	IDXGIAdapter* adapter = nullptr;
	if (FAILED(factory->EnumAdapters(0, &adapter))) return false;

	// 4. 출력(모니터)에 대한 첫번째 어댑터를 지정.
	IDXGIOutput* adapterOutput = nullptr;
	if (FAILED(adapter->EnumOutputs(0, &adapterOutput))) return false;

	// 5. 출력 (모니터)에 대한 DXGI_FORMAT_R8G8B8A8_UNORM 표시 형식에 맞는 모드 수를 얻음.
	unsigned int numModes = 0;
	if (FAILED(adapterOutput->GetDisplayModeList(
													DXGI_FORMAT_R8G8B8A8_UNORM,
													DXGI_ENUM_MODES_INTERLACED,
													&numModes,
													NULL))) 
	{
		return false;
	}

	// 6. 가능한 모든 모니터와 그래픽카드 조합을 저장할 리스트를 생성.
	DXGI_MODE_DESC* displayModeList = new DXGI_MODE_DESC[numModes];
	if (!displayModeList) return false;

	// 7. 디스플레이 모드에 대한 리스트를 채웁니다
	if (FAILED(adapterOutput->GetDisplayModeList(
													DXGI_FORMAT_R8G8B8A8_UNORM,
													DXGI_ENUM_MODES_INTERLACED,
													&numModes, displayModeList)))
	{
		return false;
	}

	//8. 모든 디스플레이 모드에 대해 화면의 너비/높이에 맞는 디스플레이 모드를 찾음.
	//   (만약 적합한 것을 찾으면, 모니터의 새로고침 비율의 분모와 분자값을 저장함.)
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
	
	// 9. 비디오 카드 구조체 얻기
	DXGI_ADAPTER_DESC adapterDesc;
	if (FAILED(adapter->GetDesc(&adapterDesc))) return false;
	
	// 10. 비디오카드 메모리 용량 단위를 메가바이트 단위로 저장.
	m_videoCardMemory = (int)(adapterDesc.DedicatedVideoMemory / 1024 / 1024);

	// 비디오카드 이름 저장.
	// TODO:: 대체할 함수 혹은 오류 수정.
	/*size_t stringLength = 0;
	if (wcstombs_s(&stringLength, m_videoCardDescription, 128, strName, 128) != 0) return false;*/

	m_videoCardDescription = adapterDesc.Description;

	// 11. 어댑터 및 팩토리 관련 변수 해제
	// 디스플레이 모드 리스트를 해제
	delete[] displayModeList;
	displayModeList = 0;

	// 출력 어뎁터를 해제
	adapterOutput->Release();
	adapterOutput = 0;

	// 어뎁터를 해제
	adapter->Release();
	adapter = 0;

	// 팩토리 객체를 해제
	factory->Release();
	factory = 0;

	// 12. 스왑체인 구조체 초기화
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	ZeroMemory(&swapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));

	// 백버퍼 갯수 지정.
	swapChainDesc.BufferCount = 1;

	//백버퍼 넓이, 높이 지정
	swapChainDesc.BufferDesc.Width = _ptResolution.x;
	swapChainDesc.BufferDesc.Height = _ptResolution.y;

	//32bit 서페이스 지정.
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;

	//백버퍼 새로고침 비율 설정(수직동기화 처리)
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

	//백버퍼 사용용도 지정.
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

	//렌더링에 사용될 윈도우 핸들 지정.
	swapChainDesc.OutputWindow = _hWnd;


	//멀티샘플링 설정
	swapChainDesc.SampleDesc.Count = 1;		//픽셀당 추출할 표본 갯수
	swapChainDesc.SampleDesc.Quality = 0;	// 원하는 품질 수준

	//창모드 or 풀스크린 설정
	if (_bFullscreen)
	{
		swapChainDesc.Windowed = false;
	}
	else
	{
		swapChainDesc.Windowed = true;
	}

	// 스캔라인 순서 및 크기를 지정하지 않음으로 설정.
	swapChainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	// 출력된 다음 백버퍼를 비우도록 지정.
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	// 추가 옵션 플래그를 사용하지 않음.
	swapChainDesc.Flags = 0;

	//피처레벨 DirectX 11로 설정.
	D3D_FEATURE_LEVEL featureLevel = D3D_FEATURE_LEVEL_11_0;

	// 13. 스왑체인, Direct3D 장치 및 Direct3D 장치 컨텍스트 생성
	if (FAILED(D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, &featureLevel, 1,
		D3D11_SDK_VERSION, &swapChainDesc, &m_pSwapChain, &m_pDevice, NULL, &m_pDeviceContext)))
	{
		return false;
	}

	// 14. RenderTargetView 초기화
	// 백버퍼 포인터를 얻어오기
	ID3D11Texture2D* backBufferPtr = nullptr;
	if (FAILED(m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBufferPtr))) return false;

	// 백 버퍼 포인터로 렌더 타겟 뷰 생성
	if (FAILED(m_pDevice->CreateRenderTargetView(backBufferPtr, NULL, &m_pRenderTargetView))) return false;

	// 백버퍼 포인터 해제
	backBufferPtr->Release();
	backBufferPtr = 0;


	// 15. 깊이 버퍼 텍스쳐 생성
	// 깊이 버퍼 구조체를 초기화
	D3D11_TEXTURE2D_DESC depthBufferDesc;
	ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));

	// 깊이 버퍼 구조체를 작성
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

	// 설정된 깊이버퍼 구조체를 사용하여 깊이 버퍼 텍스쳐를 생성
	if (FAILED(m_pDevice->CreateTexture2D(&depthBufferDesc, NULL, &m_pDepthStencilBuffer))) return false;


	// 16. 스텐실 버퍼 상태 저장.
	// 스텐실 상태 구조체를 초기화
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
	ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));

	// 스텐실 상태 구조체를 작성
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;

	depthStencilDesc.StencilEnable = true;
	depthStencilDesc.StencilReadMask = 0xFF;
	depthStencilDesc.StencilWriteMask = 0xFF;

	// 픽셀 정면의 스텐실 설정
	depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// 픽셀 뒷면의 스텐실 설정
	depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// 깊이 스텐실 상태를 생성
	if (FAILED(m_pDevice->CreateDepthStencilState(&depthStencilDesc, &m_pDepthStencilState))) return false;


	// 17. 깊이 스텐실 상태 저장.
	m_pDeviceContext->OMSetDepthStencilState(m_pDepthStencilState, 1);

	// 18. 깊이/스텐실 뷰 바인딩
	// 깊이 스텐실 뷰의 구조체를 초기화
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));

	// 깊이 스텐실 뷰 구조체를 설정
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	// 깊이 스텐실 뷰를 생성합니다
	if (FAILED(m_pDevice->CreateDepthStencilView(m_pDepthStencilBuffer, &depthStencilViewDesc, &m_pDepthStencilView)))
	{
		return false;
	}

	// 렌더링 대상 뷰와 깊이 스텐실 버퍼를 출력 렌더 파이프 라인에 바인딩
	m_pDeviceContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);


	// 19. 레스터 라이저 설정
	// 그려지는 폴리곤과 방법을 결정할 래스터 구조체를 설정
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

	// 방금 작성한 구조체에서 래스터 라이저 상태만들기
	if (FAILED(m_pDevice->CreateRasterizerState(&rasterDesc, &m_pRasterizerState)))
	{
		return false;
	}

	// 래스터 라이저 상태를 설정
	m_pDeviceContext->RSSetState(m_pRasterizerState);


	//20. 뷰포트 설정
	// 렌더링을 위해 뷰포트를 설정
	D3D11_VIEWPORT viewport;
	viewport.Width = (float)_ptResolution.x;
	viewport.Height = (float)_ptResolution.y;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;

	// 뷰포트 생성
	m_pDeviceContext->RSSetViewports(1, &viewport);


	// 21. 투영 행렬 설정 (3차원의 물체를 2차원으로 표현하기 위해 쓰는 행렬.) 
	//-> 투영과정을 거쳐야 2D모니터에 출력이 가능하기 때문에 필수 과정임.
	// 투영 행렬 설정
	// FOV(Field Of View) -> 화면에 표현할 시야의 각도를 표현하는 값. 보여주는 각도 만큼 투영시킬 화면의 범위를 넓혀줌.
	float fieldOfView = XM_PI / 4.0f;
	float screenAspect = (float)_ptResolution.x / (float)_ptResolution.y;


	// 3D 렌더링을위한 투영 행렬 만들기
	// TODO:: 수정.
	m_projectionMatrix = XMMatrixPerspectiveFovLH(fieldOfView, screenAspect, screenNear, screenDepth);


	// 22. 월드 행렬 설정
	// 월드 행렬은 모델링을 월드 좌표로 옮겨줄때 사용하는 행렬임.
	// TODO:: 공부
	// 월드 행렬을 항등 행렬로 초기화합니다
	m_worldMatrix = XMMatrixIdentity();


	// 23. 직교 투영 행렬
	// UI처럼 원근감이 필요없는 물체는 일반 투영행렬이 아닌 직교 투영행렬이 필요함. -> FOV만 빼면 됨.
	// 2D 렌더링을위한 직교 투영 행렬을 만듭니다
	// TODO:: 수정.
	m_orthoMatrix = XMMatrixOrthographicLH((float)_ptResolution.x, (float)_ptResolution.y, screenNear, screenDepth);

	return true;
}


void D3DClass::BeginRender(HDC _dc)
{
	// 버퍼를 지울 색을 설정합니다
	float color[4] = { .5f, .5f, .5f, 1.f };

	// 백버퍼를 지웁니다
	m_pDeviceContext->ClearRenderTargetView(m_pRenderTargetView, color);

	// 깊이 버퍼를 지웁니다
	m_pDeviceContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
}



void D3DClass::EndRender(HDC _dc)
{
	// 렌더링이 완료되었으므로 화면에 백 버퍼를 표시합니다.
	if (m_bVsync)
	{
		// 화면 새로 고침 비율을 고정합니다.
		m_pSwapChain->Present(1, 0);
	}
	else
	{
		// 가능한 빠르게 출력
		m_pSwapChain->Present(0, 0);
	}
}

void D3DClass::Shutdown()
{
	// 종료 전 윈도우 모드로 설정하지 않으면 스왑 체인을 해제 할 때 예외가 발생합니다.
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


