//--------------------------------------------------------------------------------------
// FullScreenQuad.h
//
// Class to draw a full-screen quad
//
// Advanced Technology Group (ATG)
// Copyright (C) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#pragma once

#include <functional>


namespace DX
{

#if defined(__d3d12_h__) || defined(__d3d12_x_h__)

    class FullScreenQuad
    {
    public:
        void Initialize(_In_ ID3D12Device* d3dDevice);
        void Draw(_In_ ID3D12GraphicsCommandList* d3dCommandList, _In_ ID3D12PipelineState* d3dPSO, D3D12_GPU_DESCRIPTOR_HANDLE texture, D3D12_GPU_VIRTUAL_ADDRESS constantBuffer = 0);

        void ReleaseDevice();

        ID3D12RootSignature* GetRootSignature() const { return m_d3dRootSignature.Get(); }

    private:
        Microsoft::WRL::ComPtr<ID3D12RootSignature> m_d3dRootSignature;
    };

#elif defined(__d3d11_h__) || defined(__d3d11_x_h__)

    class FullScreenQuad
    {
    public:
        void Initialize(_In_ ID3D11Device* d3dDevice);
        void Draw(_In_ ID3D11DeviceContext* d3dContext, DirectX::CommonStates& states, _In_ ID3D11ShaderResourceView* texture, _In_opt_ std::function<void __cdecl()> setCustomState = nullptr);
        void ReleaseDevice();

    private:
        Microsoft::WRL::ComPtr<ID3D11VertexShader>  m_vertexShader;
        Microsoft::WRL::ComPtr<ID3D11PixelShader>   m_pixelShader;
    };

#else
#   error Please #include <d3d11.h> or <d3d12.h>
#endif

}