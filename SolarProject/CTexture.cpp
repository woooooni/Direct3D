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

	//targa �̹��������͸� �޸𸮿� �ε�.
	LoadTarga(_strFilePath, width, height);

	//�ؽ����� ����ü ����
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

	//�� �ؽ��� ����
	HRESULT hResult = _pDevice->CreateTexture2D(&textureDesc, NULL, &m_texture);
	if (FAILED(hResult))
		return;

	// targa �̹����� ������ �ʺ� ������ ����
	UINT rowPitch = (width * 4) * sizeof(unsigned char);

	// targa �̹��� �����͸� �ؽ��Ŀ� ����
	_pDevCon->UpdateSubresource(m_texture, 0, NULL, m_targaData, rowPitch, 0);

	//���̴� ���ҽ� �� ����ü ����.
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = -1;

	//�ؽ����� ���̴� ���ҽ� �並 ����.
	hResult = _pDevice->CreateShaderResourceView(m_texture, &srvDesc, &m_textureView);
	if (FAILED(hResult))
		return;


	//�ؽ��� �Ӹ� ����.
	_pDevCon->GenerateMips(m_textureView);

	//�̹��� �����Ͱ� �ؽ��Ŀ� �ε�Ǿ�����, targa�̹��� �����͸� ����.
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


	//���� ������� �߿� ������ �ε�.
	width = (int)targaFileHeader.width;
	height = (int)targaFileHeader.height;
	int bpp = (int)targaFileHeader.bpp;

	//������ 32bit���� 24bit���� üũ
	if (bpp != 32)
		return;

	// 32bit �̹��� �������� ũ�� ���
	int imageSize = width * height * 4;

	//targa �̹��� �����Ϳ� �޸� �Ҵ�
	unsigned char* targaImage = new unsigned char[imageSize];
	if (!targaImage)
		return;

	//targa �̹��� ������ �б�
	count = (unsigned int)fread(targaImage, 1, imageSize, filePtr);
	if (count != imageSize)
		return;

	fclose(filePtr);

	m_targaData = new unsigned char[imageSize];
	if (!m_targaData)
		return;

	//targa �̹��� �����Ϳ� �ε����� �ʱ�ȭ
	int index = 0;
	int k = (width * height * 4) - (width * 4);

	//targa ������ �Ųٷ� ����Ǿ����Ƿ� �ùٸ� ������ targa �̹��� �����͸� targa ��� �迭�� ����
	for (int j = 0; j < height; j++)
	{
		for (int i = 0; i < width; i++)
		{
			m_targaData[index + 0] = targaImage[k + 2]; //����
			m_targaData[index + 1] = targaImage[k + 1]; //���
			m_targaData[index + 2] = targaImage[k + 0]; //�Ķ�
			m_targaData[index + 3] = targaImage[k + 3]; //����

			k += 4;
			index += 4;
		}
		k -= (width * 8);
	}

	//���迭�� ����� targa �̹��� �����͸� ����
	delete[] targaImage;


}


