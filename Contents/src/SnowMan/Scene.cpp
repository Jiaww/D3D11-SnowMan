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
	//Set timer
	m_timer.SetFixedTimeStep(true);
	m_timer.SetTargetElapsedSeconds(1.f / 60.f);

    m_gamePad = std::make_unique<GamePad>();

    m_keyboard = std::make_unique<Keyboard>();

	m_mouse = std::make_unique<Mouse>();
	m_mouse->SetWindow(window);
	m_mouse->SetMode(Mouse::MODE_RELATIVE);

	last_scroll_value = m_mouse->GetState().scrollWheelValue;
	

    m_deviceResources->SetWindow(window, width, height);

    m_deviceResources->CreateDeviceResources();  	
    CreateDeviceDependentResources();

    m_deviceResources->CreateWindowSizeDependentResources();
    CreateWindowSizeDependentResources();

	Cam.SetPosition(12.0, 10.0, -12.0);
	//Cam.SetPosition(-20.0, 20.0, -20.0); 
	m_pitch = -0.25*PI;
	m_yaw = -0.25*PI;
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
	if (kb.F) {
		if (Cam.IsOnCar)
			Cam.OffCar = true;
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
	matrix_Buffer->world = XMMatrixTranspose(component->model * worldM);
	matrix_Buffer->invTransWorld = XMMatrixInverse(nullptr, component->model * worldM);
	matrix_Buffer->view = XMMatrixTranspose(Cam.View());
	matrix_Buffer->projection = XMMatrixTranspose(Cam.Proj());
	matrix_Buffer->lightView = XMMatrixTranspose(this->lightView);
	matrix_Buffer->lightProjection = XMMatrixTranspose(this->lightProjection);
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

	float deltaTime = float(m_timer.GetElapsedSeconds());
	float frameCount = float(m_timer.GetFrameCount());
    m_deviceResources->PIXBeginEvent(L"Render");
    auto context = m_deviceResources->GetD3DDeviceContext();

    // Set input assembler state.
    context->IASetInputLayout(m_spInputLayout.Get());

    UINT strides = sizeof(VertexPositionNormalTexture);
    UINT offsets = 0;
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	auto renderTarget = m_deviceResources->GetRenderTargetView();
	auto depthStencil = m_deviceResources->GetDepthStencilView();

// Animation Obj 2 and 3
	//Box
	float deltaRot = deltaTime * XM_PI * 0.25;
	totalRot += deltaRot;
	XMMATRIX Rotation = XMMatrixRotationRollPitchYaw(0.0, totalRot, 0.0);
	//XMMATRIX Rotation = XMMatrixIdentity();
	Objs[2]->AnimM = Rotation;
	Objs[3]->AnimM = Rotation;
	//Check if in Box
	XMFLOAT3 Box[8];
	Box[0] = XMFLOAT3(carPos.x - carScale.x*0.5, carPos.y - carScale.y*0.5, carPos.z - carScale.z*0.5);
	Box[1] = XMFLOAT3(carPos.x + carScale.x*0.5, carPos.y - carScale.y*0.5, carPos.z - carScale.z*0.5);
	Box[2] = XMFLOAT3(carPos.x - carScale.x*0.5, carPos.y + carScale.y*0.5, carPos.z - carScale.z*0.5);
	Box[3] = XMFLOAT3(carPos.x - carScale.x*0.5, carPos.y - carScale.y*0.5, carPos.z + carScale.z*0.5);
	Box[4] = XMFLOAT3(carPos.x + carScale.x*0.5, carPos.y + carScale.y*0.5, carPos.z - carScale.z*0.5);
	Box[5] = XMFLOAT3(carPos.x + carScale.x*0.5, carPos.y - carScale.y*0.5, carPos.z + carScale.z*0.5);
	Box[6] = XMFLOAT3(carPos.x - carScale.x*0.5, carPos.y + carScale.y*0.5, carPos.z + carScale.z*0.5);
	Box[7] = XMFLOAT3(carPos.x + carScale.x*0.5, carPos.y + carScale.y*0.5, carPos.z + carScale.z*0.5);

	Cam.IsOnCar = IsIntersectBBox(Cam.GetPosition(), Cam.BBoxHalfWidth, Rotation, Box);
	if (Cam.IsOnCar) {
		XMFLOAT3 NewCamPos;
		if (Cam.OffCar) {
			//
			float offset = sqrt(carScale.x*carScale.x + carScale.z*carScale.z);
			XMStoreFloat3(&NewCamPos, XMVector3Transform(XMLoadFloat3(&XMFLOAT3(carPos.x + offset, carPos.y, carPos.z)), XMMatrixRotationRollPitchYaw(0.0, totalRot, 0.0)));
			Cam.SetPosition(NewCamPos.x, NewCamPos.y, NewCamPos.z);
			Cam.OffCar = false;
		}
		else {
			XMStoreFloat3(&NewCamPos, XMVector3Transform(XMLoadFloat3(&Cam.GetPosition()), XMMatrixRotationRollPitchYaw(0.0, deltaRot, 0.0)));
			Cam.SetPosition(NewCamPos.x, NewCamPos.y, NewCamPos.z);
			Cam.UpdateViewMatrix();
		}
	}
	else
		Cam.OffCar = false;


// Shadow Pass
	context->OMSetRenderTargets(1, m_shadowTargetView.GetAddressOf(), m_shadowDepthView.Get());
	float color[4] = {0.0,0.0,0.0,1.0};
	context->ClearRenderTargetView(m_shadowTargetView.Get(), color);
	context->ClearDepthStencilView(m_shadowDepthView.Get(),D3D11_CLEAR_DEPTH, 1.0f, 0);
	// Set shaders
	context->VSSetShader(m_shadowVertexShader.Get(), nullptr, 0);
	context->GSSetShader(nullptr, nullptr, 0);
	context->PSSetShader(m_shadowPixelShader.Get(), nullptr, 0);
	////Terrain
	{
		// Set Vertex Buffer
		context->IASetVertexBuffers(0, 1, &Objs[0]->geo->components[0]->vertexBuffer, &strides, &offsets);
		context->IASetIndexBuffer(Objs[0]->geo->components[0]->indexBuffer, DXGI_FORMAT_R16_UINT, 0);
		// Set the constant buffer.
		SetConstantBufferPars(Objs[0]->geo->components[0], Objs[0]->WorldM * Objs[0]->AnimM);
		context->VSSetConstantBuffers(0, 1, m_MatrixBuffer.GetAddressOf());
		context->VSSetConstantBuffers(1, 1, m_CameraBuffer.GetAddressOf());
		context->PSSetConstantBuffers(0, 1, m_ColorBuffer.GetAddressOf());
		// Draw
		context->DrawIndexed(Objs[0]->geo->components[0]->indices.size(), 0, 0);
	}
	// Render Objects
	// Start From 1
	for (int i = 1; i < Objs.size(); i++) {
		for (int j = 0; j < Objs[i]->geo->components.size(); j++) {
			// Set Vertex Buffer
			context->IASetVertexBuffers(0, 1, &Objs[i]->geo->components[j]->vertexBuffer, &strides, &offsets);
			context->IASetIndexBuffer(Objs[i]->geo->components[j]->indexBuffer, DXGI_FORMAT_R16_UINT, 0);
			
			// Set the constant buffer.
			SetConstantBufferPars(Objs[i]->geo->components[j], Objs[i]->WorldM * Objs[i]->AnimM);
			context->VSSetConstantBuffers(0, 1, m_MatrixBuffer.GetAddressOf());
			context->VSSetConstantBuffers(1, 1, m_CameraBuffer.GetAddressOf());
			context->PSSetConstantBuffers(0, 1, m_ColorBuffer.GetAddressOf());
			// Draw
			context->DrawIndexed(Objs[i]->geo->components[j]->indices.size(), 0, 0);
		}
	}

	//set render targets
	context->OMSetRenderTargets(1, &renderTarget, depthStencil);
	//context->ClearRenderTargetView(renderTargetView, D3D11Color(0.6f, 0.6f, 0.6f, 0));
	context->ClearDepthStencilView(depthStencil, D3D11_CLEAR_DEPTH, 1.0f, 0);

//Skybox
	{
		// Set Vertex Buffer
		context->IASetVertexBuffers(0, 1, &SkyBox->components[0]->vertexBuffer, &strides, &offsets);
		context->IASetIndexBuffer(SkyBox->components[0]->indexBuffer, DXGI_FORMAT_R16_UINT, 0);
		// Set shaders
		context->VSSetShader(m_skyboxVertexShader.Get(), nullptr, 0);
		context->GSSetShader(nullptr, nullptr, 0);
		context->PSSetShader(m_skyboxPixelShader.Get(), nullptr, 0);
		// Set the constant buffer.
		SetConstantBufferPars(SkyBox->components[0], XMMatrixIdentity());
		context->VSSetConstantBuffers(0, 1, m_MatrixBuffer.GetAddressOf());
		context->VSSetConstantBuffers(1, 1, m_CameraBuffer.GetAddressOf());
		context->PSSetConstantBuffers(0, 1, m_ColorBuffer.GetAddressOf());
		// Set Sampler and Tex
		if (SkyBox->components[0]->texture != nullptr) {
			// Set texture and sampler.
			auto sampler = m_spSampler.Get();
			context->PSSetSamplers(0, 1, &sampler);
			auto texture = SkyBox->components[0]->texture;
			context->PSSetShaderResources(0, 1, &texture);
		}
		// Draw
		context->DrawIndexed(SkyBox->components[0]->indices.size(), 0, 0);
	}

	//Terrain
	{
		// Set Vertex Buffer
		context->IASetVertexBuffers(0, 1, &Objs[0]->geo->components[0]->vertexBuffer, &strides, &offsets);
		context->IASetIndexBuffer(Objs[0]->geo->components[0]->indexBuffer, DXGI_FORMAT_R16_UINT, 0);
		// Set shaders
		context->VSSetShader(m_terrainVertexShader.Get(), nullptr, 0);
		context->GSSetShader(nullptr, nullptr, 0);
		context->PSSetShader(m_terrainPixelShader.Get(), nullptr, 0);
		// Set the constant buffer.
		SetConstantBufferPars(Objs[0]->geo->components[0], Objs[0]->WorldM * Objs[0]->AnimM);
		context->VSSetConstantBuffers(0, 1, m_MatrixBuffer.GetAddressOf());
		context->VSSetConstantBuffers(1, 1, m_CameraBuffer.GetAddressOf());
		context->PSSetConstantBuffers(0, 1, m_ColorBuffer.GetAddressOf());
		// Set Sampler and Tex
		if (Objs[0]->geo->components[0]->texture != nullptr) {
			// Set texture and sampler.
			auto sampler = m_spSampler.Get();
			context->PSSetSamplers(0, 1, &sampler);
			auto texture = Objs[0]->geo->components[0]->texture;
			context->PSSetShaderResources(0, 1, &texture);
		}
		// Normal Map
		if (Objs[0]->geo->components[0]->normalMap != nullptr) {
			// Set texture and sampler.
			auto sampler = m_spSampler.Get();
			context->PSSetSamplers(1, 1, &sampler);
			auto normalMap = Objs[0]->geo->components[0]->normalMap;
			context->PSSetShaderResources(1, 1, &normalMap);
		}
		// Set Shadow Map
		{
			auto sampler = m_spSampler.Get();
			context->PSSetSamplers(2, 1, &sampler);
			context->PSSetShaderResources(2, 1, m_shadowResourceView.GetAddressOf());
		}
		// Draw
		context->DrawIndexed(Objs[0]->geo->components[0]->indices.size(), 0, 0);
	}

	
	// Start From 1
	for (int i = 1; i < Objs.size(); i++) {
		for (int j = 0; j < Objs[i]->geo->components.size(); j++) {
			// Set Vertex Buffer
			context->IASetVertexBuffers(0, 1, &Objs[i]->geo->components[j]->vertexBuffer, &strides, &offsets);
			context->IASetIndexBuffer(Objs[i]->geo->components[j]->indexBuffer, DXGI_FORMAT_R16_UINT, 0);
			// Set shaders
			context->VSSetShader(m_spVertexShader.Get(), nullptr, 0);
			context->GSSetShader(nullptr, nullptr, 0);
			context->PSSetShader(m_spPixelShader.Get(), nullptr, 0);
			// Set the constant buffer.
			SetConstantBufferPars(Objs[i]->geo->components[j], Objs[i]->WorldM * Objs[i]->AnimM);
			context->VSSetConstantBuffers(0, 1, m_MatrixBuffer.GetAddressOf());
			context->VSSetConstantBuffers(1, 1, m_CameraBuffer.GetAddressOf());
			context->PSSetConstantBuffers(0, 1, m_ColorBuffer.GetAddressOf());
			// Set Sampler and Tex
			if (Objs[i]->geo->components[j]->texture != nullptr) {
				// Set texture and sampler.
				auto sampler = m_spSampler.Get();
				context->PSSetSamplers(0, 1, &sampler);
				auto texture = Objs[i]->geo->components[j]->texture;
				context->PSSetShaderResources(0, 1, &texture);
			}
			// Normal Map
			if (Objs[i]->geo->components[j]->normalMap != nullptr) {
				// Set texture and sampler.
				auto sampler = m_spSampler.Get();
				context->PSSetSamplers(1, 1, &sampler);
				auto normalMap = Objs[i]->geo->components[j]->normalMap;
				context->PSSetShaderResources(2, 1, &normalMap);
			}
			// Set Shadow Map
			{
				auto sampler = m_spSampler.Get();
				context->PSSetSamplers(2, 1, &sampler);
				context->PSSetShaderResources(2, 1, m_shadowResourceView.GetAddressOf());
			}
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

	D3D11_DEPTH_STENCIL_DESC dsDesc;

	// Depth test parameters
	dsDesc.DepthEnable = true;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;

	// Stencil test parameters
	dsDesc.StencilEnable = true;
	dsDesc.StencilReadMask = 0xFF;
	dsDesc.StencilWriteMask = 0xFF;

	// Stencil operations if pixel is front-facing
	dsDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	dsDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Stencil operations if pixel is back-facing
	dsDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	dsDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	dsDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Create depth stencil state
	ID3D11DepthStencilState * pDSState;	
	device->CreateDepthStencilState(&dsDesc, &pDSState);

   // Load and create shaders.
    auto vertexShaderBlob = DX::ReadData(L"VertexShader.cso");

    DX::ThrowIfFailed(
        device->CreateVertexShader(vertexShaderBlob.data(), vertexShaderBlob.size(),
            nullptr, m_spVertexShader.ReleaseAndGetAddressOf()));

    auto pixelShaderBlob = DX::ReadData(L"PixelShader.cso");

    DX::ThrowIfFailed(
        device->CreatePixelShader(pixelShaderBlob.data(), pixelShaderBlob.size(),
            nullptr, m_spPixelShader.ReleaseAndGetAddressOf()));

	auto skyboxVertexShaderBlob = DX::ReadData(L"skyboxVert.cso");

	DX::ThrowIfFailed(
		device->CreateVertexShader(skyboxVertexShaderBlob.data(), skyboxVertexShaderBlob.size(),
			nullptr, m_skyboxVertexShader.ReleaseAndGetAddressOf()));

	auto skyboxPixelShaderBlob = DX::ReadData(L"skyboxPixel.cso");

	DX::ThrowIfFailed(
		device->CreatePixelShader(skyboxPixelShaderBlob.data(), skyboxPixelShaderBlob.size(),
			nullptr, m_skyboxPixelShader.ReleaseAndGetAddressOf()));

	auto terrainVertexShaderBlob = DX::ReadData(L"terrainVert.cso");

	DX::ThrowIfFailed(
		device->CreateVertexShader(terrainVertexShaderBlob.data(), terrainVertexShaderBlob.size(),
			nullptr, m_terrainVertexShader.ReleaseAndGetAddressOf()));

	auto terrainPixelShaderBlob = DX::ReadData(L"terrainPixel.cso");

	DX::ThrowIfFailed(
		device->CreatePixelShader(terrainPixelShaderBlob.data(), terrainPixelShaderBlob.size(),
			nullptr, m_terrainPixelShader.ReleaseAndGetAddressOf()));

	auto shadowVertexShaderBlob = DX::ReadData(L"shadowVert.cso");

	DX::ThrowIfFailed(
		device->CreateVertexShader(shadowVertexShaderBlob.data(), shadowVertexShaderBlob.size(),
			nullptr, m_shadowVertexShader.ReleaseAndGetAddressOf()));

	auto shadowPixelShaderBlob = DX::ReadData(L"shadowPixel.cso");

	DX::ThrowIfFailed(
		device->CreatePixelShader(shadowPixelShaderBlob.data(), shadowPixelShaderBlob.size(),
			nullptr, m_shadowPixelShader.ReleaseAndGetAddressOf()));
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

	DX::ThrowIfFailed(
		device->CreateInputLayout(s_inputElementDesc, _countof(s_inputElementDesc),
			skyboxVertexShaderBlob.data(), skyboxVertexShaderBlob.size(),
			m_spInputLayout.ReleaseAndGetAddressOf()));

	DX::ThrowIfFailed(
		device->CreateInputLayout(s_inputElementDesc, _countof(s_inputElementDesc),
			terrainVertexShaderBlob.data(), terrainVertexShaderBlob.size(),
			m_spInputLayout.ReleaseAndGetAddressOf()));

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

// Create Sampler
// Create sampler.
	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	DX::ThrowIfFailed(
		device->CreateSamplerState(&samplerDesc,
			m_spSampler.ReleaseAndGetAddressOf()));

// Create Shadow Map info
	CreateRenderToTextureResources();
	XMMATRIX lightView, lightProj;
	Cam.SetPosition(-20.0, 20.0, -20.0);
	m_pitch = -0.25*PI;
	m_yaw = 0.25*PI;
	Cam.Turn(m_pitch, m_yaw);
	Cam.UpdateViewMatrix();
	lightView = Cam.View();
	lightProj = XMMatrixOrthographicLH(30.0, 30.0, Cam.GetNearZ(), Cam.GetFarZ());
	this->lightView = lightView;
	this->lightProjection = lightProj;
// Create Skybox
	this->SkyBox = new skybox();
	SkyBox->create(device);
// Create Objects of the scene
	//Terrain
	terrain* t = new terrain();
	t->terrainDim = 16;
	t->HP_filename = L"Media/terrainHM.r16";
	t->create(device);
	Objs.push_back(new Object(t, XMMatrixScaling(1.0, 1.0, 1.0) * XMMatrixTranslation(-96, 0.0, -96), XMMatrixIdentity()));
	/*plane* p = new plane();
	p->create(device);
	Objs.push_back(new Object(p, XMMatrixScaling(10.0, 10.0, 10.0), XMMatrixIdentity()));*/
	//Snowman 1
	snowMan* sm = new snowMan();
	sm->create(device);
	Objs.push_back(new Object(sm, XMMatrixTranslation(0.0, t->GetHeight(-96, -96), 0.0), XMMatrixScaling(1.5,1.5,1.5)));
	//Box
	cube* cb = new cube();
	cb->create(device);
	carPos = XMFLOAT3(8.0, 1.725, 0.0);
	carScale = XMFLOAT3(2.0, 2.0, 2.0);
	Objs.push_back(new Object(cb, XMMatrixScaling(carScale.x, carScale.y, carScale.z) * XMMatrixTranslation(carPos.x, carPos.y, carPos.z), XMMatrixIdentity()));
	//Snowman 2 
	Objs.push_back(new Object(sm,  XMMatrixTranslation(carPos.x, carPos.y + carScale.y*0.5, carPos.z), XMMatrixIdentity()));

}

void Scene::CreateRenderToTextureResources() {
	auto device = m_deviceResources->GetD3DDevice();

	// Create Shadow Map Info
	/*
	width = shadow map width
	height = shadow map height
	*/

	// Create a depth stencil view for use with 3D rendering if needed.
	//create shadow map texture desc
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	D3D11_TEXTURE2D_DESC depthBufferDesc;
	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
	D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
	D3D11_TEXTURE2D_DESC textureDesc;

	ZeroMemory(&textureDesc, sizeof(textureDesc));
	float tex_width, tex_height;
	tex_width = 1280.0;
	tex_height = 960.0;
	textureDesc.Width = tex_width;
	textureDesc.Height = tex_height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	// Create the render target texture.
	DX::ThrowIfFailed(
		device->CreateTexture2D(&textureDesc, NULL, m_renderTargetTexture.ReleaseAndGetAddressOf()));

	// Setup the description of the render target view.
	renderTargetViewDesc.Format = textureDesc.Format;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc.Texture2D.MipSlice = 0;

	// Create the render target view.
	DX::ThrowIfFailed(
		device->CreateRenderTargetView(m_renderTargetTexture.Get(), &renderTargetViewDesc, m_shadowTargetView.ReleaseAndGetAddressOf()));

	// Setup the description of the shader resource view.
	shaderResourceViewDesc.Format = textureDesc.Format;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;

	// Create the shader resource view.
	DX::ThrowIfFailed(
		device->CreateShaderResourceView(m_renderTargetTexture.Get(), &shaderResourceViewDesc, m_shadowResourceView.ReleaseAndGetAddressOf()));

	// Initialize the description of the depth buffer.
	ZeroMemory(&depthBufferDesc, sizeof(depthBufferDesc));

	// Set up the description of the depth buffer.
	depthBufferDesc.Width = tex_width;
	depthBufferDesc.Height = tex_height;
	depthBufferDesc.MipLevels = 1;
	depthBufferDesc.ArraySize = 1;
	depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthBufferDesc.SampleDesc.Count = 1;
	depthBufferDesc.SampleDesc.Quality = 0;
	depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthBufferDesc.CPUAccessFlags = 0;
	depthBufferDesc.MiscFlags = 0;

	// Create the texture for the depth buffer using the filled out description.
	DX::ThrowIfFailed(
		device->CreateTexture2D(&depthBufferDesc, NULL, m_depthStencilBuffer.ReleaseAndGetAddressOf()));

	// Initailze the depth stencil view description.
	ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));

	// Set up the depth stencil view description.
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Texture2D.MipSlice = 0;

	// Create the depth stencil view.
	DX::ThrowIfFailed(
		device->CreateDepthStencilView(m_depthStencilBuffer.Get(), &depthStencilViewDesc, m_shadowDepthView.ReleaseAndGetAddressOf()));

}

// Allocate all memory resources that change on a window SizeChanged event.
void Scene::CreateWindowSizeDependentResources()
{
}

void Scene::OnDeviceLost()
{
    m_spInputLayout.Reset();
    m_spVertexShader.Reset();
    m_spPixelShader.Reset();
	m_skyboxVertexShader.Reset();
	m_skyboxPixelShader.Reset();
	m_terrainVertexShader.Reset();
	m_terrainPixelShader.Reset();
	m_MatrixBuffer.Reset();
	m_CameraBuffer.Reset();
	m_ColorBuffer.Reset();
	m_spSampler.Reset();
}

void Scene::OnDeviceRestored()
{
    CreateDeviceDependentResources();

    CreateWindowSizeDependentResources();
}
#pragma endregion

