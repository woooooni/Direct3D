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

	//위쪽을 가리키는 벡터를 설정
	up.x = 0.f;
	up.y = 1.f;
	up.z = 0.f;

	upVec = XMLoadFloat3(&up);

	//3D월드에서 카메라의 위치를 설정.
	position = m_position;

	//XMVECTOR 구조체에 로드
	positionVec = XMLoadFloat3(&position);

	//기본적으로 카메라가 찾고있는 위치를 설정.
	lookAt.x = 0.f;
	lookAt.y = 0.f;
	lookAt.z = 1.f;

	//XMVECTOR 구조체에 로드
	lookAtVec = XMLoadFloat3(&lookAt);

	//yaw(y), pitch(x), roll(z)축의 회전값을 라디안 단위로 설정.
	pitch = m_rotation.x * 0.0174532925f;
	yaw = m_rotation.y * 0.0174532925f;
	roll = m_rotation.z * 0.0174532925f;

	//yaw(y), pitch(x), roll(z) 값을 이용해 회전 행렬을 만듦.
	rotationMat = XMMatrixRotationRollPitchYaw(pitch, yaw, roll);

	//lookAt 및 up 벡터를 회전 행렬로 변형하여 뷰가 원점에서 올바르게 회전되도록 함.
	lookAtVec = XMVectorAdd(positionVec, lookAtVec);

	//마지막으로 세개의 업데이트 된 벡터에서 뷰 행렬을 만듦.
	m_viewMatrix = XMMatrixLookAtLH(positionVec, lookAtVec, upVec);
}
