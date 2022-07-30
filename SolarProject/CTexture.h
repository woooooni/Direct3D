#pragma once
class CTexture
{
private:
	struct TargaHeader
	{
		unsigned char data1[12];
		unsigned short width;
		unsigned short height;
		unsigned char bpp;
		unsigned char data2;
	};

private:
	ID3D11ShaderResourceView* m_textureView;
	unsigned char* m_targaData;
	ID3D11Texture2D* m_texture;


public:
	void init(ID3D11Device* _pDevice, ID3D11DeviceContext* _pDevCon, const wchar_t* _strFilePath);
	void Clear();

private:
	void LoadTarga(const wchar_t* _strFilePath, int& width, int& height);

public:
	ID3D11ShaderResourceView* GetTexture() { return m_textureView; }


public:
	CTexture();
	~CTexture();

};

