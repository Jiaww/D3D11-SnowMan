#include "pch.h"
#include "Camera.h"

using namespace DirectX;

Camera::Camera()
	: position(0.0f, 0.0f, 0.0f),
	right(1.0f, 0.0f, 0.0f),
	up(0.0f, 1.0f, 0.0f),
	look(0.0f, 0.0f, 1.0f)
{
	SetLens(XMConvertToRadians(45.0f), 1280.0f/960.f, 0.1f, 1000.0f);
}

Camera::~Camera()
{
}

XMVECTOR Camera::GetPositionXM()const
{
	return XMLoadFloat3(&position);
}

XMFLOAT3 Camera::GetPosition()const
{
	return position;
}

void Camera::SetPosition(float x, float y, float z)
{
	position = XMFLOAT3(x, y, z);
}

void Camera::SetPosition(const XMFLOAT3& v)
{
	position = v;
}

XMVECTOR Camera::GetRightXM()const
{
	return XMLoadFloat3(&right);
}

XMFLOAT3 Camera::GetRight()const
{
	return right;
}

XMVECTOR Camera::GetUpXM()const
{
	return XMLoadFloat3(&up);
}

XMFLOAT3 Camera::GetUp()const
{
	return up;
}

XMVECTOR Camera::GetLookXM()const
{
	return XMLoadFloat3(&look);
}

XMFLOAT3 Camera::GetLook()const
{
	return look;
}

float Camera::GetNearZ()const
{
	return near_clip;
}

float Camera::GetFarZ()const
{
	return far_clip;
}

float Camera::GetAspect()const
{
	return aspect;
}

float Camera::GetFovY()const
{
	return fovy;
}

float Camera::GetFovX()const
{
	float halfWidth = 0.5f*GetNearWindowWidth();
	return 2.0f*atan(halfWidth / near_clip);
}

float Camera::GetNearWindowWidth()const
{
	return aspect * nearWindowHeight;
}

float Camera::GetNearWindowHeight()const
{
	return nearWindowHeight;
}

float Camera::GetFarWindowWidth()const
{
	return aspect * farWindowHeight;
}

float Camera::GetFarWindowHeight()const
{
	return farWindowHeight;
}

void Camera::SetLens(float fovY, float aspect, float zn, float zf)
{
	// cache properties
	fovy = fovY;
	aspect = aspect;
	near_clip = zn;
	far_clip = zf;

	nearWindowHeight = 2.0f * near_clip * tanf(0.5f*fovy);
	farWindowHeight = 2.0f * far_clip * tanf(0.5f*fovy);

	XMMATRIX P = XMMatrixPerspectiveFovLH(fovy, aspect, near_clip, far_clip);
	XMStoreFloat4x4(&projection, P);
}

void Camera::LookAt(FXMVECTOR pos, FXMVECTOR target, FXMVECTOR worldUp)
{
	XMVECTOR L = XMVector3Normalize(XMVectorSubtract(target, pos));
	XMVECTOR R = XMVector3Normalize(XMVector3Cross(worldUp, L));
	XMVECTOR U = XMVector3Cross(L, R);

	XMStoreFloat3(&position, pos);
	XMStoreFloat3(&look, L);
	XMStoreFloat3(&right, R);
	XMStoreFloat3(&up, U);
}

void Camera::LookAt(const XMFLOAT3& pos, const XMFLOAT3& target, const XMFLOAT3& up)
{
	XMVECTOR P = XMLoadFloat3(&pos);
	XMVECTOR T = XMLoadFloat3(&target);
	XMVECTOR U = XMLoadFloat3(&up);

	LookAt(P, T, U);
}

XMMATRIX Camera::View()const
{
	return XMLoadFloat4x4(&view);
}

XMMATRIX Camera::Proj()const
{
	return XMLoadFloat4x4(&projection);
}

XMMATRIX Camera::ViewProj()const
{
	return XMMatrixMultiply(View(), Proj());
}

void Camera::Strafe(float d)
{
	// position += d*right
	XMVECTOR s = XMVectorReplicate(d);
	XMVECTOR r = XMLoadFloat3(&right);
	XMVECTOR p = XMLoadFloat3(&position);
	XMStoreFloat3(&position, XMVectorMultiplyAdd(s, r, p));
}

void Camera::Walk(float d)
{
	// position += d*look
	XMVECTOR s = XMVectorReplicate(d);
	XMVECTOR l = XMLoadFloat3(&look);
	XMVECTOR p = XMLoadFloat3(&position);
	XMStoreFloat3(&position, XMVectorMultiplyAdd(s, l, p));
}

void Camera::Turn(float pitch, float yaw) {
	float y = sinf(pitch);
	float r = cosf(pitch);
	float z = r * cosf(yaw);
	float x = r * sinf(yaw);
	XMFLOAT3 target = DirectX::XMFLOAT3(position.x + x, position.y + y, position.z + z);
	LookAt(position, target, DirectX::XMFLOAT3(0.0, 1.0, 0.0));
}

void Camera::Pitch(float angle)
{
	// Rotate up and look vector about the right vector.

	XMMATRIX R = XMMatrixRotationAxis(XMLoadFloat3(&right), angle);

	XMStoreFloat3(&up, XMVector3TransformNormal(XMLoadFloat3(&up), R));
	XMStoreFloat3(&look, XMVector3TransformNormal(XMLoadFloat3(&look), R));
}

void Camera::RotateY(float angle)
{
	// Rotate the basis vectors about the world y-axis.

	XMMATRIX R = XMMatrixRotationY(angle);

	XMStoreFloat3(&right, XMVector3TransformNormal(XMLoadFloat3(&right), R));
	XMStoreFloat3(&up, XMVector3TransformNormal(XMLoadFloat3(&up), R));
	XMStoreFloat3(&look, XMVector3TransformNormal(XMLoadFloat3(&look), R));
}

void Camera::UpdateViewMatrix()
{
	XMVECTOR R = XMLoadFloat3(&right);
	XMVECTOR U = XMLoadFloat3(&up);
	XMVECTOR L = XMLoadFloat3(&look);
	XMVECTOR P = XMLoadFloat3(&position);

	// Keep camera's axes orthogonal to each other and of unit length.
	L = XMVector3Normalize(L);
	U = XMVector3Normalize(XMVector3Cross(L, R));

	// U, L already ortho-normal, so no need to normalize cross product.
	R = XMVector3Cross(U, L);

	// Fill in the view matrix entries.
	float x = -XMVectorGetX(XMVector3Dot(P, R));
	float y = -XMVectorGetX(XMVector3Dot(P, U));
	float z = -XMVectorGetX(XMVector3Dot(P, L));

	XMStoreFloat3(&right, R);
	XMStoreFloat3(&up, U);
	XMStoreFloat3(&look, L);

	view(0, 0) = right.x;
	view(1, 0) = right.y;
	view(2, 0) = right.z;
	view(3, 0) = x;

	view(0, 1) = up.x;
	view(1, 1) = up.y;
	view(2, 1) = up.z;
	view(3, 1) = y;

	view(0, 2) = look.x;
	view(1, 2) = look.y;
	view(2, 2) = look.z;
	view(3, 2) = z;

	view(0, 3) = 0.0f;
	view(1, 3) = 0.0f;
	view(2, 3) = 0.0f;
	view(3, 3) = 1.0f;
}

