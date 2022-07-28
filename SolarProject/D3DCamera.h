#pragma once
class D3DCamera : public AlignedAllocationPolicy<16>
{
private:
	XMFLOAT3 m_position;
	XMFLOAT3 m_rotation;
	XMMATRIX m_viewMatrix;

public:
	XMFLOAT3 GetPosition() { return m_position; }
	XMFLOAT3 GetRotation() { return m_rotation; }
	void SetPosition(float x, float y, float z) { m_position.x = x; m_position.y = y; m_position.z = z; }
	void SetRotation(float x, float y, float z) { m_rotation.x = x; m_rotation.y = y; m_rotation.z = z; }
	
	XMMATRIX GetViewMatrix() { return m_viewMatrix; }

public:
	void render();

public:
	D3DCamera();
	~D3DCamera();
};

