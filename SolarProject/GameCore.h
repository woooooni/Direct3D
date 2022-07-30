#pragma once
#include "pch.h"
#include "D3DClass.h"

class D3DCamera;
class D3DModel;
class D3DColorShader;
class D3DTextureShader;
class GameCore
{
public:
	SINGLETON(GameCore);
private:
	
	HWND				m_hWnd = NULL;	// 메인 윈도우 핸들
	HDC					m_hDC;			// 메인 윈도우 DC
	POINT				m_ptResolution;	// 메인 윈도우 해상도
	
	HBRUSH				m_arrBrush[(UINT)BRUSH_TYPE::END];
	HPEN				m_arrPen[(UINT)PEN_TYPE::END];

	D3DCamera*			m_3dCam;
	D3DModel*			m_3dModel;
	D3DColorShader*		m_3dColorShader;
	D3DTextureShader*	m_3dTextureShader;
public:
	int init(HWND _hWnd, POINT _ptResolution);
	void progress();

private:
	void Clear();

public:
	HWND GetMainHwnd() { return m_hWnd; }
	HDC GetMainDC() { return m_hDC; }
	POINT GetResolution() { return m_ptResolution; }
};

