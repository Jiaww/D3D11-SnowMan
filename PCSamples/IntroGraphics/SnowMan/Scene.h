//--------------------------------------------------------------------------------------
// SimpleTrianglePC.h
//
// Advanced Technology Group (ATG)
// Copyright (C) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#pragma once
  	
#include "DeviceResources.h"
#include "StepTimer.h"
#include "RModel.h"
#include "Camera.h"
#include <GeometricPrimitive.h>
#include "Utilities.h"
#include "snowMan.h"
#include "plane.h"

struct Object {
	drawable* geo;
	DirectX::XMMATRIX WorldM;
	Object(drawable* g, DirectX::XMMATRIX wm) :geo(g), WorldM(wm) {}
};

// A basic sample implementation that creates a D3D11 device and
// provides a render loop.
class Scene : public DX::IDeviceNotify
{
public:

    Scene();

    // Initialization and management
    void Initialize(HWND window, int width, int height);

    // Basic render loop
    void Tick();

    // IDeviceNotify
    virtual void OnDeviceLost() override;
    virtual void OnDeviceRestored() override;

    // Messages
    void OnActivated();
    void OnDeactivated();
    void OnSuspending();
    void OnResuming();
    void OnWindowSizeChanged(int width, int height);

    // Properties
    void GetDefaultSize( int& width, int& height ) const;

	Camera Cam;
private:

    void Update(DX::StepTimer const& timer);
    void Render();

    void Clear();

    void CreateDeviceDependentResources();
    void CreateWindowSizeDependentResources();

	void SetConstantBufferPars(const RModel* component, DirectX::XMMATRIX worldM);

    // Device resources.
    std::unique_ptr<DX::DeviceResources>    m_deviceResources;

    // Rendering loop timer.
    DX::StepTimer                           m_timer;

    // Input devices.
    std::unique_ptr<DirectX::GamePad>       m_gamePad;
    std::unique_ptr<DirectX::Keyboard>      m_keyboard;
	std::unique_ptr<DirectX::Mouse>			m_mouse;
	DirectX::Mouse::ButtonStateTracker				m_tracker;
	float last_scroll_value;
	float m_pitch, m_yaw;

    // Scene objects
    Microsoft::WRL::ComPtr<ID3D11InputLayout>       m_spInputLayout;
	Microsoft::WRL::ComPtr<ID3D11Buffer>            m_spVertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer>            m_spIndexBuffer;

	std::vector<Object*> Objs;

	Microsoft::WRL::ComPtr<ID3D11VertexShader>      m_spVertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader>       m_spPixelShader;

	std::vector<DirectX::VertexPositionNormalTexture> vertices;
	std::vector<uint16_t> indices;

	Microsoft::WRL::ComPtr<ID3D11Buffer> m_MatrixBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_CameraBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_ColorBuffer;
};