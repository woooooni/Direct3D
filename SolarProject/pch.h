#pragma once

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

#define WIN32_LEAN_AND_MEAN					// 거의 사용되지 않는 내용은 Windows헤더에서 제외.
#include "targetver.h"
#include <Windows.h>
#include <wtypes.h>
#include <iostream>
#include <memory.h>
#include <tchar.h>
#include <stdlib.h>
#include <crtdbg.h>

#include <d3d11.h>
#include <d3dtypes.h>
#include <d3dcompiler.h>
#include <d3dcaps.h>



#include<string>
using namespace std;

#include<DirectXMath.h>
using namespace DirectX;


#include "define.h"
//warning C4316처리
#include "AlignedAllocationPolicy.h"