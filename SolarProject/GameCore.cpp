#include "pch.h"
#include "GameCore.h"
#include "D3DCamera.h"
#include "D3DModel.h"
#include "D3DColorShader.h"


GameCore::GameCore()
	: m_3dCam(nullptr)
	, m_3dModel(nullptr)
	, m_3dShader(nullptr)
{
	
}

GameCore::~GameCore()
{
	if (m_3dCam != nullptr)
		delete m_3dCam;

	if (m_3dModel != nullptr)
		delete m_3dModel;

	if (m_3dShader != nullptr)
		delete m_3dShader;
}

int GameCore::init(HWND _hWnd, POINT _ptResolution)
{
	m_hWnd = _hWnd;
	m_ptResolution = _ptResolution;
	m_hDC = GetDC(_hWnd);

	if (!D3DClass::GetInst()->Initialize(m_hWnd, m_ptResolution, VSYNC_ENABLED, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR))
	{
		MessageBox(_hWnd, L"Could not initialize Direct3D.", L"Error", MB_OK);
		return S_FALSE;
	}
	

	m_3dCam = new D3DCamera;
	m_3dModel = new D3DModel;
	m_3dShader = new D3DColorShader;


	m_3dCam->SetPosition(0.f, 0.f, -5.f);
	m_3dModel->init(D3DClass::GetInst()->GetDevice());
	m_3dShader->init(D3DClass::GetInst()->GetDevice(), m_hWnd);


	return S_OK;
}

void GameCore::progress()
{
	D3DClass::GetInst()->BeginRender(m_hDC);
	m_3dCam->render();
	
	XMMATRIX world, view, projection;
	world = D3DClass::GetInst()->GetWorldMatrix();
	view = m_3dCam->GetViewMatrix();
	projection = D3DClass::GetInst()->GetProjectionMatrix();

	m_3dModel->render(D3DClass::GetInst()->GetDeviceContext());

	m_3dShader->render(D3DClass::GetInst()->GetDeviceContext(), m_3dModel->GetIndexCount(), world, view, projection);
	D3DClass::GetInst()->EndRender(m_hDC);
}

void GameCore::Clear()
{
	
}
