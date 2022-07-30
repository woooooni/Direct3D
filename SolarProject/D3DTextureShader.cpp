#include "pch.h"
#include "D3DTextureShader.h"
D3DTextureShader::D3DTextureShader()
	: m_vertexShader(nullptr)
	, m_pixelShader(nullptr)
	, m_layout(nullptr)
	, m_matrixBuffer(nullptr)
	, m_sampleState(nullptr)
{
}

D3DTextureShader::~D3DTextureShader()
{
}


void D3DTextureShader::init(ID3D11Device* _pDevice, HWND _hWnd)
{
	wchar_t strVsPath[255] = {};
	GetCurrentDirectory(255, strVsPath);
	wcscat_s(strVsPath, 255, L"\\TextureVS.hlsl");

	wchar_t strPsPath[255] = {};
	GetCurrentDirectory(255, strPsPath);
	wcscat_s(strPsPath, 255, L"\\TexturePS.hlsl");
	InitializeShader(_pDevice, _hWnd, strVsPath, strPsPath);
}

void D3DTextureShader::render(ID3D11DeviceContext* _pDevCon, int _index, XMMATRIX _worldMat, XMMATRIX _viewMat, XMMATRIX _projMat, ID3D11ShaderResourceView* _pResource)
{
	SetShaderParameters(_pDevCon, _worldMat, _viewMat, _projMat, _pResource);
	RenderShader(_pDevCon, _index);
}

void D3DTextureShader::Clear()
{
	ShutDownShader();
}

bool D3DTextureShader::InitializeShader(ID3D11Device* _pDevice, HWND _hWnd, const wchar_t* _vsFileName, const wchar_t* _psFileName)
{
	HRESULT result;
	ID3DBlob* errMsg = nullptr;

	//버텍스 쉐이더 코드를 컴파일
	ID3D10Blob* vertexShaderBuffer = nullptr;
	result = D3DCompileFromFile(
		_vsFileName, NULL, NULL, "TextureVertexShader", "vs_5_0",
		D3D10_SHADER_ENABLE_STRICTNESS, 0, &vertexShaderBuffer, &errMsg);

	if (FAILED(result))
	{
		if (errMsg)
		{
			OutputShaderErrorMessage(errMsg, _hWnd, _vsFileName);
		}
		else
		{
			MessageBox(_hWnd, _vsFileName, L"Missing Shader File", MB_OK);
		}
		return false;
	}

	//픽셀 쉐이더 코드를 컴파일
	ID3D10Blob* pixelShaderBuffer = nullptr;
	result = D3DCompileFromFile(
		_psFileName, NULL, NULL, "TexturePixelShader", "ps_5_0",
		D3D10_SHADER_ENABLE_STRICTNESS, 0, &pixelShaderBuffer, &errMsg);
	if (FAILED(result))
	{
		if (errMsg)
		{
			OutputShaderErrorMessage(errMsg, _hWnd, _vsFileName);
		}
		else
		{
			MessageBox(_hWnd, _vsFileName, L"Missing Shader File", MB_OK);
			return false;
		}
		return false;
	}

	// 버퍼로부터 정점 셰이더를 생성.
	//TODO :: 오류 수정(파일 안불러와짐 혹은 dll 오류)
	result = _pDevice->
		CreateVertexShader(vertexShaderBuffer->GetBufferPointer()
			, vertexShaderBuffer->GetBufferSize()
			, NULL
			, &m_vertexShader);
	if (FAILED(result))
	{
		MessageBox(_hWnd, _vsFileName, L"Failed Create Vertex Shader", MB_OK);
		return false;
	}

	// 버퍼로부터 픽셀 쉐이더를 생성.
	result = _pDevice->
		CreatePixelShader(pixelShaderBuffer->GetBufferPointer()
			, pixelShaderBuffer->GetBufferSize()
			, NULL
			, &m_pixelShader);
	if (FAILED(result))
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

	polygonLayout[1].SemanticName = "TEXCOORD";
	polygonLayout[1].SemanticIndex = 0;
	polygonLayout[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	polygonLayout[1].InputSlot = 0;
	polygonLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[1].InstanceDataStepRate = 0;

	// 레이아웃의 요소 수를 가져옵니다.
	unsigned int numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

	// 정점 입력 레이아웃을 만듭니다.
	result = _pDevice->CreateInputLayout(polygonLayout, numElements,
		vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), &m_layout);
	if (FAILED(result))
	{
		return false;
	}

	// 더 이상 사용되지 않는 정점 셰이더 퍼버와 픽셀 셰이더 버퍼를 해제.
	Safe_Release(vertexShaderBuffer);
	Safe_Release(pixelShaderBuffer);
	
	// 정점 셰이더에 있는 행렬 상수 버퍼의 구조체를 작성
	D3D11_BUFFER_DESC matrixBufferDesc;
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER; matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;

	// 상수 버퍼 포인터를 만들어 이 클래스에서 정점 셰이더 상수 버퍼에 접근할 수 있게 합니다.
	result = _pDevice->CreateBuffer(&matrixBufferDesc, NULL, &m_matrixBuffer);
	if (FAILED(result))
	{
		return false;
	}

	D3D11_SAMPLER_DESC samplerDesc;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	result = _pDevice->CreateSamplerState(&samplerDesc, &m_sampleState);
	if (FAILED(result))
	{
		return false;
	}

	return true;
}

void D3DTextureShader::ShutDownShader()
{
	Safe_Release(m_matrixBuffer);
	Safe_Release(m_layout);
	Safe_Release(m_pixelShader);
	Safe_Release(m_vertexShader);
	Safe_Release(m_sampleState);
}

void D3DTextureShader::OutputShaderErrorMessage(ID3D10Blob* _pErr, HWND _hWnd, const wchar_t* _shaderFileName)
{
	OutputDebugStringA(reinterpret_cast<const char*>(_pErr->GetBufferPointer()));

	Safe_Release(_pErr);
	MessageBox(_hWnd, _shaderFileName, L"Error Compile Shader", MB_OK);
}

bool D3DTextureShader::SetShaderParameters(ID3D11DeviceContext* _pDevCon, XMMATRIX _worldMat, XMMATRIX _viewMat, XMMATRIX _projMat, ID3D11ShaderResourceView* _pResource)
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

	_pDevCon->PSGetShaderResources(0, 1, &_pResource);
	return true;
}

void D3DTextureShader::RenderShader(ID3D11DeviceContext* _pDevCon, int _index)
{
	// 정점 입력 레이아웃을 설정합니다.
	_pDevCon->IASetInputLayout(m_layout);

	// 삼각형을 그릴 정점 셰이더와 픽셀 셰이더를 설정합니다.
	_pDevCon->VSSetShader(m_vertexShader, NULL, 0);
	_pDevCon->PSSetShader(m_pixelShader, NULL, 0);

	//픽셀 쉐이더에서 샘플러 상태를 결정
	_pDevCon->PSSetSamplers(0, 1, &m_sampleState);

	// 삼각형을 그립니다.
	_pDevCon->DrawIndexed(_index, 0, 0);
}

