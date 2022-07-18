#pragma once
#include "pch.h"
class GameCore
{
public:
	SINGLETON(GameCore);
private:
	HWND	m_hWnd = NULL;	// ���� ������ �ڵ�
	HDC		m_hDC;			// ���� ������ DC
	POINT	m_ptResolution;	// ���� ������ �ػ�
	
	HBRUSH	m_arrBrush[(UINT)BRUSH_TYPE::END];
	HPEN	m_arrPen[(UINT)PEN_TYPE::END];

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

