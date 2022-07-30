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

	//���ؽ� ���̴� �ڵ带 ������
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

	//�ȼ� ���̴� �ڵ带 ������
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

	// ���۷κ��� ���� ���̴��� ����.
	//TODO :: ���� ����(���� �Ⱥҷ����� Ȥ�� dll ����)
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

	// ���۷κ��� �ȼ� ���̴��� ����.
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

	// ���� �Է� ���̾ƿ� ����ü�� �����մϴ�.
	// �� ������ ModelClass�� ���̴��� VertexType ������ ��ġ�ؾ���.
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

	// ���̾ƿ��� ��� ���� �����ɴϴ�.
	unsigned int numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

	// ���� �Է� ���̾ƿ��� ����ϴ�.
	result = _pDevice->CreateInputLayout(polygonLayout, numElements,
		vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), &m_layout);
	if (FAILED(result))
	{
		return false;
	}

	// �� �̻� ������ �ʴ� ���� ���̴� �۹��� �ȼ� ���̴� ���۸� ����.
	Safe_Release(vertexShaderBuffer);
	Safe_Release(pixelShaderBuffer);
	
	// ���� ���̴��� �ִ� ��� ��� ������ ����ü�� �ۼ�
	D3D11_BUFFER_DESC matrixBufferDesc;
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER; matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;

	// ��� ���� �����͸� ����� �� Ŭ�������� ���� ���̴� ��� ���ۿ� ������ �� �ְ� �մϴ�.
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
	// ����� transpose�Ͽ� ���̴����� ����� �� �ְ� ��.
	_worldMat = XMMatrixTranspose(_worldMat);
	_viewMat = XMMatrixTranspose(_viewMat);
	_projMat = XMMatrixTranspose(_projMat);

	// ��� ������ ������ �� �� �ֵ��� ���.
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	if (FAILED(_pDevCon->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)))
	{
		return false;
	}

	// ��� ������ �����Ϳ� ���� �����͸� ������.
	MatrixBufferType* dataPtr = (MatrixBufferType*)mappedResource.pData;

	// ��� ���ۿ� ����� ����.
	dataPtr->world = _worldMat;
	dataPtr->view = _viewMat;
	dataPtr->projection = _projMat;

	// ��� ������ ����� ǰ.
	_pDevCon->Unmap(m_matrixBuffer, 0);

	// ���� ���̴������� ��� ������ ��ġ�� ����.
	unsigned bufferNumber = 0;

	// ���������� ���� ���̴��� ��� ���۸� �ٲ� ������ �ٲٱ�.
	_pDevCon->VSSetConstantBuffers(bufferNumber, 1, &m_matrixBuffer);

	_pDevCon->PSGetShaderResources(0, 1, &_pResource);
	return true;
}

void D3DTextureShader::RenderShader(ID3D11DeviceContext* _pDevCon, int _index)
{
	// ���� �Է� ���̾ƿ��� �����մϴ�.
	_pDevCon->IASetInputLayout(m_layout);

	// �ﰢ���� �׸� ���� ���̴��� �ȼ� ���̴��� �����մϴ�.
	_pDevCon->VSSetShader(m_vertexShader, NULL, 0);
	_pDevCon->PSSetShader(m_pixelShader, NULL, 0);

	//�ȼ� ���̴����� ���÷� ���¸� ����
	_pDevCon->PSSetSamplers(0, 1, &m_sampleState);

	// �ﰢ���� �׸��ϴ�.
	_pDevCon->DrawIndexed(_index, 0, 0);
}

