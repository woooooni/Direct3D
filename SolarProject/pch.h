#pragma once

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

#define WIN32_LEAN_AND_MEAN					// ���� ������ �ʴ� ������ Windows������� ����.
#include "targetver.h"
#include <Windows.h>
#include <wtypes.h>
#include <iostream>
#include <memory.h>
#include <tchar.h>
#include <stdlib.h>
#include <crtdbg.h>
#include "define.h"
#include <d3d9.h>
#include <d3d11.h>
#include<d3dcompiler.h>
#include<d3dcaps.h>

#include<string>
using namespace std;

#include<DirectXMath.h>
using namespace DirectX;


//warning C4316ó��
#include "AlignedAllocationPolicy.h"