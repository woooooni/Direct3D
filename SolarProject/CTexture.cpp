#include "pch.h"
#include "CTexture.h"

CTexture::CTexture()
	: m_textureView(nullptr)
	, m_targaData{}
	, m_texture(nullptr)
{
}

CTexture::~CTexture()
{
	Clear();
}

void CTexture::init(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDevCon, const wchar_t* _strFilePath)
{
	int width = 0;
	int height = 0;

	//targa 이미지데이터를 메모리에 로드.
	LoadTarga(_strFilePath, width, height);

	//텍스쳐의 구조체 생성
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = width;
	textureDesc.Height = height;
	textureDesc.MipLevels = 0;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

	//빈 텍스쳐 생성
	HRESULT hResult = _pDevice->CreateTexture2D(&textureDesc, NULL, &m_texture);
	if (FAILED(hResult))
		return;

	// targa 이미지의 데이터 너비 사이즈 설정
	UINT rowPitch = (width * 4) * sizeof(unsigned char);

	// targa 이미지 데이터를 텍스쳐에 복사
	_pDevCon->UpdateSubresource(m_texture, 0, NULL, m_targaData, rowPitch, 0);

	//셰이더 리소스 뷰 구조체 설정.
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = -1;

	//텍스쳐의 셰이더 리소스 뷰를 생성.
	hResult = _pDevice->CreateShaderResourceView(m_texture, &srvDesc, &m_textureView);
	if (FAILED(hResult))
		return;


	//텍스쳐 밉맵 생성.
	_pDevCon->GenerateMips(m_textureView);

	//이미지 데이터가 텍스쳐에 로드되었으니, targa이미지 데이터를 해제.
	delete[] m_targaData;
}

void CTexture::Clear()
{
	Safe_Release(m_textureView);
	Safe_Release(m_texture);
	delete[] m_targaData;
}

void CTexture::LoadTarga(const wchar_t* _strFilePath, int& width, int& height)
{
	FILE* filePtr;
	if (_wfopen_s(&filePtr, _strFilePath, L"rb") != 0)
		return;

	TargaHeader targaFileHeader;
	unsigned int count = (unsigned int)fread(&targaFileHeader, sizeof(targaFileHeader), 1, filePtr);
	if (count != 1)
		return;


	//파일 헤더에서 중요 정보를 로드.
	width = (int)targaFileHeader.width;
	height = (int)targaFileHeader.height;
	int bpp = (int)targaFileHeader.bpp;

	//파일이 32bit인지 24bit인지 체크
	if (bpp != 32)
		return;

	// 32bit 이미지 데이터의 크기 계산
	int imageSize = width * height * 4;

	//targa 이미지 데이터용 메모리 할당
	unsigned char* targaImage = new unsigned char[imageSize];
	if (!targaImage)
		return;

	//targa 이미지 데이터 읽기
	count = (unsigned int)fread(targaImage, 1, imageSize, filePtr);
	if (count != imageSize)
		return;

	fclose(filePtr);

	m_targaData = new unsigned char[imageSize];
	if (!m_targaData)
		return;

	//targa 이미지 데이터에 인덱스를 초기화
	int index = 0;
	int k = (width * height * 4) - (width * 4);

	//targa 형식은 거꾸로 저장되었으므로 올바른 순서로 targa 이미지 데이터를 targa 대상 배열에 복사
	for (int j = 0; j < height; j++)
	{
		for (int i = 0; i < width; i++)
		{
			m_targaData[index + 0] = targaImage[k + 2]; //빨강
			m_targaData[index + 1] = targaImage[k + 1]; //녹색
			m_targaData[index + 2] = targaImage[k + 0]; //파랑
			m_targaData[index + 3] = targaImage[k + 3]; //알파

			k += 4;
			index += 4;
		}
		k -= (width * 8);
	}

	//대상배열에 복사된 targa 이미지 데이터를 해제
	delete[] targaImage;


}


