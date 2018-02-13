#pragma once
#include "pch.h"

// Thanks for the examples of Introduction-to-3D-Game-Programming-With-DirectX11 
class Camera
{
private:
	DirectX::XMFLOAT4X4 projection;
	DirectX::XMFLOAT4X4 view;
	float fovy = 45;
	float aspect = 1.0;
	float near_clip = 0.1;
	float far_clip = 1000;
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 right;
	DirectX::XMFLOAT3 up;
	DirectX::XMFLOAT3 look;
	float nearWindowHeight;
	float farWindowHeight;

public:
	Camera();
	~Camera();

	bool IsOnCar = false;
	DirectX::XMMATRIX AnimM;
	bool OffCar = false;
	DirectX::XMFLOAT3 BBoxHalfWidth = DirectX::XMFLOAT3(1.0f,1.0f,1.0f);

	// Get/Set world camera position.
	DirectX::XMVECTOR GetPositionXM()const;
	DirectX::XMFLOAT3 GetPosition();
	void SetPosition(float x, float y, float z);
	void SetPosition(const DirectX::XMFLOAT3& v);

	// Get camera basis vectors.
	DirectX::XMVECTOR GetRightXM()const;
	DirectX::XMFLOAT3 GetRight()const;
	DirectX::XMVECTOR GetUpXM()const;
	DirectX::XMFLOAT3 GetUp()const;
	DirectX::XMVECTOR GetLookXM()const;
	DirectX::XMFLOAT3 GetLook()const;

	// Get frustum properties.
	float GetNearZ()const;
	float GetFarZ()const;
	float GetAspect()const;
	float GetFovY()const;
	float GetFovX()const;

	// Get near and far plane dimensions in view space coordinates.
	float GetNearWindowWidth()const;
	float GetNearWindowHeight()const;
	float GetFarWindowWidth()const;
	float GetFarWindowHeight()const;

	// Set frustum.
	void SetLens(float fovY, float aspect, float zn, float zf);

	// Define camera space via LookAt parameters.
	void LookAt(DirectX::FXMVECTOR pos, DirectX::FXMVECTOR target, DirectX::FXMVECTOR worldUp);
	void LookAt(const DirectX::XMFLOAT3& pos, const DirectX::XMFLOAT3& target, const DirectX::XMFLOAT3& up);

	// Get View/Proj matrices.
	DirectX::XMMATRIX View()const;
	DirectX::XMMATRIX Proj()const;
	DirectX::XMMATRIX ViewProj()const;

	// Strafe/Walk the camera a distance d.
	void Strafe(float d);
	void Walk(float d);

	// Rotate the camera.
	void Turn(float pitch, float yaw);
	void Pitch(float angle);
	void RotateY(float angle);

	// After modifying camera position/orientation, call to rebuild the view matrix.
	void UpdateViewMatrix();
};

