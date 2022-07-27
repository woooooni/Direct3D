#include "GameCore.h"


GameCore::GameCore()
{
	
}

GameCore::~GameCore()
{
	D3DClass::GetInst()->Shutdown();
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
	return S_OK;
}

void GameCore::progress()
{
	D3DClass::GetInst()->BeginRender(m_hDC);
	D3DClass::GetInst()->EndRender(m_hDC);
}

void GameCore::Clear()
{
	
}
