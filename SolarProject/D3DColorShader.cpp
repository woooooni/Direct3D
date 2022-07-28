#include "pch.h"
#include "D3DColorShader.h"

D3DColorShader::D3DColorShader()
	: m_vertexShader(nullptr)
	, m_pixelShader(nullptr)
	, m_layout(nullptr)
	, m_matrixBuffer(nullptr)
{
}

D3DColorShader::~D3DColorShader()
{
}

void D3DColorShader::init(ID3D11Device* _pDevice, HWND _hWnd)
{
	wchar_t strVsPath[255] = {};
	GetCurrentDirectory(255, strVsPath);
	wcscat_s(strVsPath, 255, L"\\color.vs");

	wchar_t strPsPath[255] = {};
	GetCurrentDirectory(255, strPsPath);
	wcscat_s(strPsPath, 255, L"\\color.ps");
	
	InitializeShader(_pDevice, _hWnd, strVsPath, strPsPath);
}

void D3DColorShader::render(ID3D11DeviceContext* _pDevCon, int _index, XMMATRIX _worldMat, XMMATRIX _viewMat, XMMATRIX _projMat)
{
	SetShaderParameters(_pDevCon, _worldMat, _viewMat, _projMat);
	RenderShader(_pDevCon, _index);
}

void D3DColorShader::Clear()
{
	ShutDownShader();
}

bool D3DColorShader::InitializeShader(ID3D11Device* _pDevice, HWND _hWnd, const wchar_t* _vsFileName, const wchar_t* _psFileName)
{
	ID3D10Blob* errMsg = nullptr;

	//버텍스 쉐이더 코드를 컴파일
	ID3D10Blob* vertexShaderBuffer = nullptr;
	HRESULT result = D3DCompileFromFile(
		_vsFileName, NULL, NULL, "ColorVertexShader", "vs_5_0",
		D3D10_SHADER_ENABLE_STRICTNESS, 0, &vertexShaderBuffer, &errMsg);

	if (FAILED(D3DCompileFromFile(
		_vsFileName, NULL, NULL, "ColorVertexShader", "vs_5_0",
		D3D10_SHADER_ENABLE_STRICTNESS, 0, &vertexShaderBuffer, &errMsg)))
	{
		OutputShaderErrorMessage(errMsg, _hWnd, _vsFileName);
	}
	else
	{
		MessageBox(_hWnd, _vsFileName, L"Missing Shader File", MB_OK);
	}
	
	//픽셀 쉐이더 코드를 컴파일
	ID3D10Blob* pixelShaderBuffer = nullptr;
	if (FAILED(D3DCompileFromFile(
		_psFileName, NULL, NULL, "ColorPixelShader", "ps_5_0",
		D3D10_SHADER_ENABLE_STRICTNESS, 0, &pixelShaderBuffer, &errMsg)))
	{
		OutputShaderErrorMessage(errMsg, _hWnd, _vsFileName);
	}
	else
	{
		MessageBox(_hWnd, _vsFileName, L"Missing Shader File", MB_OK);
	}


	// 버퍼로부터 정점 셰이더를 생성.
	//TODO :: 오류 수정(파일 안불러와짐 혹은 dll 오류)
	if (FAILED(_pDevice->
		CreateVertexShader(vertexShaderBuffer->GetBufferPointer()
			, vertexShaderBuffer->GetBufferSize()
			, NULL
			, &m_vertexShader)))
	{
		MessageBox(_hWnd, _vsFileName, L"Failed Create Vertex Shader", MB_OK);
		return false;
	}

	// 버퍼로부터 픽셀 쉐이더를 생성.
	if (FAILED(_pDevice->
		CreatePixelShader(pixelShaderBuffer->GetBufferPointer()
							, pixelShaderBuffer->GetBufferSize()
							, NULL
							, &m_pixelShader)))
	{
		MessageBox(_hWnd, _vsFileName, L"Failed Create Pixel Shader", MB_OK);
		return false;
	}


	// 정점 입력 레이아웃 구조체를 설정합니다.
	// 이 설정은 ModelClass와 셰이더의 VertexType 구조와 일치해야함.
	D3D11_INPUT_ELEMENT_DESC polygonLayout[2];
	polygonLayout[0].SemanticName = "POSITION";
	polygonLayout[0].SemanticIndex = 0;
	polygonLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[0].InputSlot = 0;
	polygonLayout[0].AlignedByteOffset = 0;
	polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[0].InstanceDataStepRate = 0;

	polygonLayout[1].SemanticName = "COLOR";
	polygonLayout[1].SemanticIndex = 0;
	polygonLayout[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	polygonLayout[1].InputSlot = 0;
	polygonLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[1].InstanceDataStepRate = 0;

	// 레이아웃의 요소 수를 가져옵니다.
	unsigned int numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

	// 정점 입력 레이아웃을 만듭니다.
	if (FAILED(_pDevice->CreateInputLayout(polygonLayout, numElements,
		vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), &m_layout)))
	{
		return false;
	}

	// 더 이상 사용되지 않는 정점 셰이더 퍼버와 픽셀 셰이더 버퍼를 해제.
	vertexShaderBuffer->Release();
	vertexShaderBuffer = 0;

	pixelShaderBuffer->Release();
	pixelShaderBuffer = 0;

	// 정점 셰이더에 있는 행렬 상수 버퍼의 구조체를 작성
	D3D11_BUFFER_DESC matrixBufferDesc;
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER; matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;

	// 상수 버퍼 포인터를 만들어 이 클래스에서 정점 셰이더 상수 버퍼에 접근할 수 있게 합니다.
	if (FAILED(_pDevice->CreateBuffer(&matrixBufferDesc, NULL, &m_matrixBuffer)))
	{
		return false;
	}

	return true;
}

void D3DColorShader::ShutDownShader()
{
	if (nullptr != m_matrixBuffer) 
	{
		m_matrixBuffer->Release();
		m_matrixBuffer = 0;
	}

	if (nullptr != m_layout)
	{
		m_layout->Release();
		m_layout = 0;
	}

	if (nullptr != m_pixelShader)
	{
		m_pixelShader->Release();
		m_pixelShader = 0;
	}

	if (nullptr != m_vertexShader)
	{
		m_vertexShader->Release();
		m_vertexShader = 0;
	}

}

void D3DColorShader::OutputShaderErrorMessage(ID3D10Blob* _pErr, HWND _hWnd, const wchar_t* _shaderFileName)
{
	OutputDebugStringA(reinterpret_cast<const char*>(_pErr->GetBufferPointer()));

	_pErr->Release();
	_pErr = 0;

	MessageBox(_hWnd, _shaderFileName, L"Error Compile Shader", MB_OK);

}

bool D3DColorShader::SetShaderParameters(ID3D11DeviceContext* _pDevCon, XMMATRIX _worldMat, XMMATRIX _viewMat, XMMATRIX _projMat)
{
	// 행렬을 transpose하여 셰이더에서 사용할 수 있게 함.
	_worldMat = XMMatrixTranspose(_worldMat);
	_viewMat = XMMatrixTranspose(_viewMat);
	_projMat = XMMatrixTranspose(_projMat);

	// 상수 버퍼의 내용을 쓸 수 있도록 잠금.
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	if (FAILED(_pDevCon->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
	{
		return false;
	}

	// 상수 버퍼의 데이터에 대한 포인터를 가져옴.
	MatrixBufferType* dataPtr = (MatrixBufferType*)mappedResource.pData;

	// 상수 버퍼에 행렬을 복사.
	dataPtr->world = _worldMat;
	dataPtr->view = _viewMat;
	dataPtr->projection = _projMat;

	// 상수 버퍼의 잠금을 품.
	_pDevCon->Unmap(m_matrixBuffer, 0);

	// 정점 셰이더에서의 상수 버퍼의 위치를 설정.
	unsigned bufferNumber = 0;

	// 마지막으로 정점 셰이더의 상수 버퍼를 바뀐 값으로 바꾸기.
	_pDevCon->VSSetConstantBuffers(bufferNumber, 1, &m_matrixBuffer);
	return true;
}

void D3DColorShader::RenderShader(ID3D11DeviceContext* _pDevCon, int _index)
{
	// 정점 입력 레이아웃을 설정합니다.
	_pDevCon->IASetInputLayout(m_layout);

	// 삼각형을 그릴 정점 셰이더와 픽셀 셰이더를 설정합니다.
	_pDevCon->VSSetShader(m_vertexShader, NULL, 0);
	_pDevCon->PSSetShader(m_pixelShader, NULL, 0);

	// 삼각형을 그립니다.
	_pDevCon->DrawIndexed(_index, 0, 0);
}
