//--------------------------------------------------------------------------------------
// SimpleTrianglePC.cpp
//
// Advanced Technology Group (ATG)
// Copyright (C) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#include "pch.h"
#include "Scene.h"

#include "ATGColors.h"
#include "ReadData.h"

extern void ExitSample();

using namespace DirectX;

using Microsoft::WRL::ComPtr;

Scene::Scene()
{
    // Use gamma-correct rendering.
    m_deviceResources = std::make_unique<DX::DeviceResources>(DXGI_FORMAT_B8G8R8A8_UNORM_SRGB);
    m_deviceResources->RegisterDeviceNotify(this);
}

// Initialize the Direct3D resources required to run.
void Scene::Initialize(HWND window, int width, int height)
{
    m_gamePad = std::make_unique<GamePad>();

    m_keyboard = std::make_unique<Keyboard>();

	m_mouse = std::make_unique<Mouse>();
	m_mouse->SetWindow(window);
	m_mouse->SetMode(Mouse::MODE_RELATIVE);

	last_scroll_value = m_mouse->GetState().scrollWheelValue;
	m_pitch = 0;
	m_yaw = 0;

    m_deviceResources->SetWindow(window, width, height);

    m_deviceResources->CreateDeviceResources();  	
    CreateDeviceDependentResources();

    m_deviceResources->CreateWindowSizeDependentResources();
    CreateWindowSizeDependentResources();

	Cam.SetPosition(0.0, 1.0, -5.0);
	//m_pitch -= XM_PI * 0.25;
	Cam.Turn(m_pitch, m_yaw);
	Cam.UpdateViewMatrix();

}

#pragma region Frame Update
// Executes basic render loop.
void Scene::Tick()
{
    m_timer.Tick([&]()
    {
        Update(m_timer);
    });

	Cam.UpdateViewMatrix();
    Render();
}

// Updates the world.
void Scene::Update(DX::StepTimer const&)
{
    auto pad = m_gamePad->GetState(0);
    if (pad.IsConnected())
    {
        if (pad.IsViewPressed())
        {
            ExitSample();
        }
    }

    auto kb = m_keyboard->GetState();
    if (kb.Escape)
    {
        ExitSample();
    }
	if (kb.W) {
		Cam.Walk(0.05);
	}
	if (kb.A) {
		Cam.Strafe(-0.05);
	}
	if (kb.S) {
		Cam.Walk(-0.05);
	}
	if (kb.D) {
		Cam.Strafe(0.05);
	}
	if (kb.Q) {
		m_yaw -= 0.02;
	}
	if (kb.E) {
		m_yaw += 0.02;
	}
	if (kb.Z) {
		m_pitch += 0.02;
	}
	if (kb.C) {
		m_pitch -= 0.02;
	}
	auto mouse = m_mouse->GetState();
	//Scroll
	float move_forward = mouse.scrollWheelValue - last_scroll_value;
	last_scroll_value = mouse.scrollWheelValue;
	Cam.Walk(move_forward * 0.001);
	if (mouse.positionMode == Mouse::MODE_RELATIVE)
	{
		float deltaX = float(mouse.x) * 0.01;
		float deltaY = float(mouse.y) * 0.01;
		m_pitch -= deltaY;
		m_yaw += deltaX;
	}
	// limit pitch to straight up or straight down
	// with a little fudge-factor to avoid gimbal lock
	float limit = XM_PI / 2.0f - 0.01f;
	m_pitch = std::max(-limit, m_pitch);
	m_pitch = std::min(+limit, m_pitch);

	// keep longitude in sane range by wrapping
	if (m_yaw > XM_PI)
	{
		m_yaw -= XM_PI * 2.0f;
	}
	else if (m_yaw < -XM_PI)
	{
		m_yaw += XM_PI * 2.0f;
	}
	Cam.Turn(m_pitch, m_yaw);
	m_mouse->SetMode(mouse.leftButton ? Mouse::MODE_RELATIVE : Mouse::MODE_ABSOLUTE);
}
#pragma endregion

#pragma region Frame Render

void Scene::SetConstantBufferPars(const RModel* component, XMMATRIX worldM) {
	auto context = m_deviceResources->GetD3DDeviceContext();
	// Constant Buffer
	// Map Constant Buffer and Buffer Binding
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	// Set Constant Buffer
	MatrixBufferType* matrix_Buffer;
	// Map Buffer to Write
	DX::ThrowIfFailed(
		context->Map(m_MatrixBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)
	);
	matrix_Buffer = (MatrixBufferType*)mappedResource.pData;
	matrix_Buffer->world = XMMatrixTranspose(component->model) * worldM;
	matrix_Buffer->view = XMMatrixTranspose(Cam.View());
	matrix_Buffer->projection = XMMatrixTranspose(Cam.Proj());
	// Unlock the constant buffer.
	context->Unmap(m_MatrixBuffer.Get(), 0);

	CameraBufferType* camera_Buffer;
	// Map Buffer to Write
	DX::ThrowIfFailed(
		context->Map(m_CameraBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)
	);
	camera_Buffer = (CameraBufferType*)mappedResource.pData;
	camera_Buffer->camPos = XMFLOAT4(Cam.GetPosition().x, Cam.GetPosition().y, Cam.GetPosition().z, 1.0);
	// Unlock the constant buffer.
	context->Unmap(m_CameraBuffer.Get(), 0);

	ColorBufferType* color_Buffer;
	// Map Buffer to Write
	DX::ThrowIfFailed(
		context->Map(m_ColorBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource)
	);
	color_Buffer = (ColorBufferType*)mappedResource.pData;
	color_Buffer->c_color = component->color;
	// Unlock the constant buffer.
	context->Unmap(m_ColorBuffer.Get(), 0);
}

// Draws the scene.
void Scene::Render()
{
    // Don't try to render anything before the first Update.
    if (m_timer.GetFrameCount() == 0)
    {
        return;
    }

    Clear();

    m_deviceResources->PIXBeginEvent(L"Render");
    auto context = m_deviceResources->GetD3DDeviceContext();

    // Set input assembler state.
    context->IASetInputLayout(m_spInputLayout.Get());

    UINT strides = sizeof(VertexPositionNormalTexture);
    UINT offsets = 0;
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	for (int i = 0; i < Objs.size(); i++) {
		for (int j = 0; j < Objs[i]->geo->components.size(); j++) {
			// Set Vertex Buffer
			context->IASetVertexBuffers(0, 1, &Objs[i]->geo->components[j]->vertexBuffer, &strides, &offsets);
			context->IASetIndexBuffer(Objs[i]->geo->components[j]->indexBuffer, DXGI_FORMAT_R16_UINT, 0);
			// Set shaders
			context->VSSetShader(m_spVertexShader.Get(), nullptr, 0);
			context->GSSetShader(nullptr, nullptr, 0);
			context->PSSetShader(m_spPixelShader.Get(), nullptr, 0);
			// Set the constant buffer.
			SetConstantBufferPars(Objs[i]->geo->components[j], Objs[i]->WorldM);
			context->VSSetConstantBuffers(0, 1, m_MatrixBuffer.GetAddressOf());
			context->VSSetConstantBuffers(1, 1, m_CameraBuffer.GetAddressOf());
			context->PSSetConstantBuffers(0, 1, m_ColorBuffer.GetAddressOf());
			// Draw
			context->DrawIndexed(Objs[i]->geo->components[j]->indices.size(), 0, 0);
		}
	}

    m_deviceResources->PIXEndEvent();
    // Show the new frame.
    m_deviceResources->Present();
}

// Helper method to clear the back buffers.
void Scene::Clear()
{
    m_deviceResources->PIXBeginEvent(L"Clear");

    // Clear the views.
    auto context = m_deviceResources->GetD3DDeviceContext();
    auto renderTarget = m_deviceResources->GetRenderTargetView();
    auto depthStencil = m_deviceResources->GetDepthStencilView();

    // Use linear clear color for gamma-correct rendering.
    context->ClearRenderTargetView(renderTarget, ATG::ColorsLinear::Background);

    context->ClearDepthStencilView(depthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    context->OMSetRenderTargets(1, &renderTarget, depthStencil);

    // Set the viewport.
    auto viewport = m_deviceResources->GetScreenViewport();
    context->RSSetViewports(1, &viewport);

    m_deviceResources->PIXEndEvent();
}
#pragma endregion

#pragma region Message Handlers
// Message handlers
void Scene::OnActivated()
{
}

void Scene::OnDeactivated()
{
}

void Scene::OnSuspending()
{
}

void Scene::OnResuming()
{
    m_timer.ResetElapsedTime();
}

void Scene::OnWindowSizeChanged(int width, int height)
{
    if (!m_deviceResources->WindowSizeChanged(width, height))
        return;

    CreateWindowSizeDependentResources();
}

// Properties
void Scene::GetDefaultSize(int& width, int& height) const
{
    width = 1280;
    height = 960;
}
#pragma endregion

#pragma region Direct3D Resources
// These are the resources that depend on the device.
void Scene::CreateDeviceDependentResources()
{
    auto device = m_deviceResources->GetD3DDevice();

   // Load and create shaders.
    auto vertexShaderBlob = DX::ReadData(L"VertexShader.cso");

    DX::ThrowIfFailed(
        device->CreateVertexShader(vertexShaderBlob.data(), vertexShaderBlob.size(),
            nullptr, m_spVertexShader.ReleaseAndGetAddressOf()));

    auto pixelShaderBlob = DX::ReadData(L"PixelShader.cso");

    DX::ThrowIfFailed(
        device->CreatePixelShader(pixelShaderBlob.data(), pixelShaderBlob.size(),
            nullptr, m_spPixelShader.ReleaseAndGetAddressOf()));

   // Create input layout.
    static const D3D11_INPUT_ELEMENT_DESC s_inputElementDesc[3] =
    {
        { "vs_Pos", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,  D3D11_INPUT_PER_VERTEX_DATA,  0 },
        { "vs_Nor", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA , 0 },
		{ "vs_Tex", 0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA , 0 }
    };

    DX::ThrowIfFailed(
        device->CreateInputLayout(s_inputElementDesc, _countof(s_inputElementDesc),
            vertexShaderBlob.data(), vertexShaderBlob.size(),
            m_spInputLayout.ReleaseAndGetAddressOf()));
//// Vertex Buffer and Index Buffer can be create in each model, not in the shader program
//	// Create Meshes
//	GeometricPrimitive::CreateCube(vertices, indices, 2.0);
//	// Because GeometricPrimitive create un-clockwise faces
//	indices = reverseIndices(indices);
//	// Create vertex buffer.
//
//	//static const Vertex s_vertexData[3] =
// //   {
//	//	{ { 0.0f,  0.5f,  0.5f},{ 1.0f, 0.0f, 0.0f},{1.0f, 0.0f} },  // Top / Red
//	//	{ { 0.5f, -0.5f,  0.5f},{ 0.0f, 1.0f, 0.0f},{1.0f, 1.0f} },  // Right / Green
//	//	{ { -0.5f, -0.5f,  0.5f},{ 0.0f, 0.0f, 1.0f},{0.0f, 1.0f} }   // Left / Blue
//	//};
//
//	//static const uint16_t s_indexData[3] = { 0,1,2 };
//
//    D3D11_BUFFER_DESC vertexBufferDesc = { 0 };
//	vertexBufferDesc.ByteWidth = sizeof(VertexPositionNormalTexture) * vertices.size();
//	vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
//	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
//	vertexBufferDesc.StructureByteStride = 0;
//
//	D3D11_SUBRESOURCE_DATA vertexData = { 0 };
//	vertexData.pSysMem = vertices.data();
//	vertexData.SysMemPitch = 0;
//	vertexData.SysMemSlicePitch = 0;
//
//    DX::ThrowIfFailed(
//        device->CreateBuffer(&vertexBufferDesc, &vertexData,
//            m_spVertexBuffer.ReleaseAndGetAddressOf()));
//	
//	// Create Index buffer.
//
//	// Set up the description of the static index buffer.
//	D3D11_BUFFER_DESC indexBufferDesc = { 0 };
//	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
//	indexBufferDesc.ByteWidth = sizeof(uint16_t) * indices.size();
//	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
//	indexBufferDesc.CPUAccessFlags = 0;
//	indexBufferDesc.MiscFlags = 0;
//	indexBufferDesc.StructureByteStride = 0;
//
//	// Give the subresource structure a pointer to the index data.
//	D3D11_SUBRESOURCE_DATA indexData = { 0 };
//	indexData.pSysMem = indices.data();
//	indexData.SysMemPitch = 0;
//	indexData.SysMemSlicePitch = 0;
//
//	DX::ThrowIfFailed(
//		device->CreateBuffer(&indexBufferDesc, &indexData,
//			m_spIndexBuffer.ReleaseAndGetAddressOf()));
//
	// Create Constant buffer
	// Fill in a buffer description.
	D3D11_BUFFER_DESC cbDesc;
	cbDesc.ByteWidth = sizeof(MatrixBufferType);
	cbDesc.Usage = D3D11_USAGE_DYNAMIC;
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbDesc.MiscFlags = 0;
	cbDesc.StructureByteStride = 0;
	// Create the buffer.
	DX::ThrowIfFailed(
		m_deviceResources->GetD3DDevice()->CreateBuffer(&cbDesc, NULL, m_MatrixBuffer.GetAddressOf())
	);

	D3D11_BUFFER_DESC camera_cbDesc;
	camera_cbDesc.ByteWidth = sizeof(CameraBufferType);
	camera_cbDesc.Usage = D3D11_USAGE_DYNAMIC;
	camera_cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	camera_cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	camera_cbDesc.MiscFlags = 0;
	camera_cbDesc.StructureByteStride = 0;
	// Create the buffer.
	DX::ThrowIfFailed(
		m_deviceResources->GetD3DDevice()->CreateBuffer(&camera_cbDesc, NULL, m_CameraBuffer.GetAddressOf())
	);

	D3D11_BUFFER_DESC color_cbDesc;
	color_cbDesc.ByteWidth = sizeof(ColorBufferType);
	color_cbDesc.Usage = D3D11_USAGE_DYNAMIC;
	color_cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	color_cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	color_cbDesc.MiscFlags = 0;
	color_cbDesc.StructureByteStride = 0;
	// Create the buffer.
	DX::ThrowIfFailed(
		m_deviceResources->GetD3DDevice()->CreateBuffer(&color_cbDesc, NULL, m_ColorBuffer.GetAddressOf())
	);

// Create Objects
	//Terrain
	plane* p = new plane();
	p->create(device);
	Objs.push_back(new Object(p, XMMatrixIdentity()));
	//Snowman 1
	snowMan* sm1 = new snowMan();
	sm1->create(device);
	Objs.push_back(new Object(sm1, XMMatrixIdentity()));
}

// Allocate all memory resources that change on a window SizeChanged event.
void Scene::CreateWindowSizeDependentResources()
{
}

void Scene::OnDeviceLost()
{
    m_spInputLayout.Reset();
    m_spVertexBuffer.Reset();
    m_spVertexShader.Reset();
    m_spPixelShader.Reset();
}

void Scene::OnDeviceRestored()
{
    CreateDeviceDependentResources();

    CreateWindowSizeDependentResources();
}
#pragma endregion

