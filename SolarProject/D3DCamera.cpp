#include "pch.h"
#include "D3DCamera.h"

D3DCamera::D3DCamera()
	: m_position{}
	, m_rotation{}
	, m_viewMatrix{}
{
	m_position = XMFLOAT3(0.f, 0.f, 0.f);
	m_rotation = XMFLOAT3(0.f, 0.f, 0.f);
}

D3DCamera::~D3DCamera()
{
}

void D3DCamera::render()
{
	XMFLOAT3 up, position, lookAt;
	XMVECTOR upVec, positionVec, lookAtVec;
	float yaw, pitch, roll;
	XMMATRIX rotationMat;

	//������ ����Ű�� ���͸� ����
	up.x = 0.f;
	up.y = 1.f;
	up.z = 0.f;

	upVec = XMLoadFloat3(&up);

	//3D���忡�� ī�޶��� ��ġ�� ����.
	position = m_position;

	//XMVECTOR ����ü�� �ε�
	positionVec = XMLoadFloat3(&position);

	//�⺻������ ī�޶� ã���ִ� ��ġ�� ����.
	lookAt.x = 0.f;
	lookAt.y = 0.f;
	lookAt.z = 1.f;

	//XMVECTOR ����ü�� �ε�
	lookAtVec = XMLoadFloat3(&lookAt);

	//yaw(y), pitch(x), roll(z)���� ȸ������ ���� ������ ����.
	pitch = m_rotation.x * 0.0174532925f;
	yaw = m_rotation.y * 0.0174532925f;
	roll = m_rotation.z * 0.0174532925f;

	//yaw(y), pitch(x), roll(z) ���� �̿��� ȸ�� ����� ����.
	rotationMat = XMMatrixRotationRollPitchYaw(pitch, yaw, roll);

	//lookAt �� up ���͸� ȸ�� ��ķ� �����Ͽ� �䰡 �������� �ùٸ��� ȸ���ǵ��� ��.
	lookAtVec = XMVectorAdd(positionVec, lookAtVec);

	//���������� ������ ������Ʈ �� ���Ϳ��� �� ����� ����.
	m_viewMatrix = XMMatrixLookAtLH(positionVec, lookAtVec, upVec);
}
