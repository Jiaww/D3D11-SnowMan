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
#include "cube.h"
#include "skybox.h"
#include "terrain.h"

struct Object {
	drawable* geo;
	DirectX::XMMATRIX WorldM;
	DirectX::XMMATRIX AnimM;
	Object(drawable* g, DirectX::XMMATRIX wm, DirectX::XMMATRIX am) :geo(g), WorldM(wm), AnimM(am) {}
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
	void CreateRenderToTextureResources();

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

	std::vector<Object*> Objs;
	skybox* SkyBox;
	float totalRot = 0.0f;
	DirectX::XMFLOAT3 carPos;
	DirectX::XMFLOAT3 carScale;

	Microsoft::WRL::ComPtr<ID3D11VertexShader>      m_spVertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader>       m_spPixelShader;
	Microsoft::WRL::ComPtr<ID3D11VertexShader>      m_skyboxVertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>       m_skyboxPixelShader;
	Microsoft::WRL::ComPtr<ID3D11VertexShader>      m_terrainVertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>       m_terrainPixelShader;
	Microsoft::WRL::ComPtr<ID3D11VertexShader>      m_shadowVertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>       m_shadowPixelShader;

	std::vector<DirectX::VertexPositionNormalTexture> vertices;
	std::vector<uint16_t> indices;

	DirectX::XMMATRIX lightView;
	DirectX::XMMATRIX lightProjection;

	Microsoft::WRL::ComPtr<ID3D11Buffer> m_MatrixBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_CameraBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> m_ColorBuffer;


	Microsoft::WRL::ComPtr<ID3D11SamplerState>          m_spSampler;
	Microsoft::WRL::ComPtr<ID3D11Texture2D>          m_renderTargetTexture;
	Microsoft::WRL::ComPtr<ID3D11Texture2D>          m_depthStencilBuffer;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView>          m_shadowDepthView;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>          m_shadowResourceView;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_shadowTargetView;
};