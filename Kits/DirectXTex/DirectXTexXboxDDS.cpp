//--------------------------------------------------------------------------------------
// File: DirectXTexXboxDDS.cpp
//
// DirectXTex Auxillary functions for saving "XBOX" Xbox One variants of DDS files
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//--------------------------------------------------------------------------------------

#include "DirectXTexP.h"
#include "DirectXTexXbox.h"

#include "dds.h"
#include "xdk.h"

using namespace DirectX;
using namespace Xbox;

namespace
{
    const DDS_PIXELFORMAT DDSPF_XBOX =
    { sizeof(DDS_PIXELFORMAT), DDS_FOURCC, MAKEFOURCC('X','B','O','X'), 0, 0, 0, 0, 0 };

#pragma pack(push,1)

    struct DDS_HEADER_XBOX
        // Must match structure in XboxDDSTextureLoader module
    {
        DXGI_FORMAT dxgiFormat;
        uint32_t    resourceDimension;
        uint32_t    miscFlag; // see DDS_RESOURCE_MISC_FLAG
        uint32_t    arraySize;
        uint32_t    miscFlags2; // see DDS_MISC_FLAGS2
        uint32_t    tileMode; // see XG_TILE_MODE
        uint32_t    baseAlignment;
        uint32_t    dataSize;
        uint32_t    xdkVer; // matching _XDK_VER
    };

#pragma pack(pop)

    static_assert(sizeof(DDS_HEADER_XBOX) == 36, "DDS XBOX Header size mismatch");
    static_assert(sizeof(DDS_HEADER_XBOX) >= sizeof(DDS_HEADER_DXT10), "DDS XBOX Header should be larger than DX10 header");

    static const size_t XBOX_HEADER_SIZE = sizeof(uint32_t) + sizeof(DDS_HEADER) + sizeof(DDS_HEADER_XBOX);

    //-------------------------------------------------------------------------------------
    // Decodes DDS header using XBOX extended header (variant of DX10 header)
    //-------------------------------------------------------------------------------------
    HRESULT DecodeDDSHeader(
        _In_reads_bytes_(size) const void* pSource,
        size_t size,
        DirectX::TexMetadata& metadata,
        _Out_opt_ XG_TILE_MODE* tmode,
        _Out_opt_ uint32_t* dataSize,
        _Out_opt_ uint32_t* baseAlignment)
    {
        if (!pSource)
            return E_INVALIDARG;

        if (tmode)
            *tmode = XG_TILE_MODE_INVALID;
        if (dataSize)
            *dataSize = 0;
        if (baseAlignment)
            *baseAlignment = 0;

        memset(&metadata, 0, sizeof(TexMetadata));

        if (size < (sizeof(DDS_HEADER) + sizeof(uint32_t)))
        {
            return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
        }

        // DDS files always start with the same magic number ("DDS ")
        uint32_t dwMagicNumber = *reinterpret_cast<const uint32_t*>(pSource);
        if (dwMagicNumber != DDS_MAGIC)
        {
            return E_FAIL;
        }

        auto pHeader = reinterpret_cast<const DDS_HEADER*>(reinterpret_cast<const uint8_t*>(pSource) + sizeof(uint32_t));

        // Verify header to validate DDS file
        if (pHeader->dwSize != sizeof(DDS_HEADER)
            || pHeader->ddspf.dwSize != sizeof(DDS_PIXELFORMAT))
        {
            return E_FAIL;
        }

        metadata.mipLevels = pHeader->dwMipMapCount;
        if (metadata.mipLevels == 0)
            metadata.mipLevels = 1;

        // Check for XBOX extension
        if (!(pHeader->ddspf.dwFlags & DDS_FOURCC)
            || (MAKEFOURCC('X', 'B', 'O', 'X') != pHeader->ddspf.dwFourCC))
        {
            // We know it's a DDS file, but it's not an XBOX extension
            return S_FALSE;
        }

        // Buffer must be big enough for both headers and magic value
        if (size < XBOX_HEADER_SIZE)
        {
            return E_FAIL;
        }

        auto xboxext = reinterpret_cast<const DDS_HEADER_XBOX*>(
            reinterpret_cast<const uint8_t*>(pSource) + sizeof(uint32_t) + sizeof(DDS_HEADER));

        if (xboxext->xdkVer < _XDK_VER)
        {
            OutputDebugStringA("WARNING: DDS XBOX file may be outdated and need regeneration\n");
        }

        metadata.arraySize = xboxext->arraySize;
        if (metadata.arraySize == 0)
        {
            return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
        }

        metadata.format = xboxext->dxgiFormat;
        if (!IsValid(metadata.format))
        {
            return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
        }

        static_assert(TEX_MISC_TEXTURECUBE == DDS_RESOURCE_MISC_TEXTURECUBE, "DDS header mismatch");

        metadata.miscFlags = xboxext->miscFlag & ~TEX_MISC_TEXTURECUBE;

        switch (xboxext->resourceDimension)
        {
        case DDS_DIMENSION_TEXTURE1D:

            if ((pHeader->dwFlags & DDS_HEIGHT) && pHeader->dwHeight != 1)
            {
                return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
            }

            metadata.width = pHeader->dwWidth;
            metadata.height = 1;
            metadata.depth = 1;
            metadata.dimension = TEX_DIMENSION_TEXTURE1D;
            break;

        case DDS_DIMENSION_TEXTURE2D:
            if (xboxext->miscFlag & DDS_RESOURCE_MISC_TEXTURECUBE)
            {
                metadata.miscFlags |= TEX_MISC_TEXTURECUBE;
                metadata.arraySize *= 6;
            }

            metadata.width = pHeader->dwWidth;
            metadata.height = pHeader->dwHeight;
            metadata.depth = 1;
            metadata.dimension = TEX_DIMENSION_TEXTURE2D;
            break;

        case DDS_DIMENSION_TEXTURE3D:
            if (!(pHeader->dwFlags & DDS_HEADER_FLAGS_VOLUME))
            {
                return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
            }

            if (metadata.arraySize > 1)
                return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);

            metadata.width = pHeader->dwWidth;
            metadata.height = pHeader->dwHeight;
            metadata.depth = pHeader->dwDepth;
            metadata.dimension = TEX_DIMENSION_TEXTURE3D;
            break;

        default:
            return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
        }

        static_assert(TEX_MISC2_ALPHA_MODE_MASK == DDS_MISC_FLAGS2_ALPHA_MODE_MASK, "DDS header mismatch");

        static_assert(TEX_ALPHA_MODE_UNKNOWN == DDS_ALPHA_MODE_UNKNOWN, "DDS header mismatch");
        static_assert(TEX_ALPHA_MODE_STRAIGHT == DDS_ALPHA_MODE_STRAIGHT, "DDS header mismatch");
        static_assert(TEX_ALPHA_MODE_PREMULTIPLIED == DDS_ALPHA_MODE_PREMULTIPLIED, "DDS header mismatch");
        static_assert(TEX_ALPHA_MODE_OPAQUE == DDS_ALPHA_MODE_OPAQUE, "DDS header mismatch");
        static_assert(TEX_ALPHA_MODE_CUSTOM == DDS_ALPHA_MODE_CUSTOM, "DDS header mismatch");

        metadata.miscFlags2 = xboxext->miscFlags2;

        if (tmode)
            *tmode = static_cast<XG_TILE_MODE>(xboxext->tileMode);

        if (baseAlignment)
            *baseAlignment = xboxext->baseAlignment;

        if (dataSize)
            *dataSize = xboxext->dataSize;

        return S_OK;
    }


    //-------------------------------------------------------------------------------------
    // Encodes DDS file header (magic value, header, XBOX extended header)
    //-------------------------------------------------------------------------------------
    HRESULT EncodeDDSHeader(
        const XboxImage& xbox,
        _Out_writes_(maxsize) void* pDestination,
        size_t maxsize)
    {
        if (!pDestination)
            return E_POINTER;

        if (maxsize < XBOX_HEADER_SIZE)
            return E_NOT_SUFFICIENT_BUFFER;

        *reinterpret_cast<uint32_t*>(pDestination) = DDS_MAGIC;

        auto header = reinterpret_cast<DDS_HEADER*>(reinterpret_cast<uint8_t*>(pDestination) + sizeof(uint32_t));

        memset(header, 0, sizeof(DDS_HEADER));
        header->dwSize = sizeof(DDS_HEADER);
        header->dwFlags = DDS_HEADER_FLAGS_TEXTURE;
        header->dwCaps = DDS_SURFACE_FLAGS_TEXTURE;

        auto& metadata = xbox.GetMetadata();

        if (metadata.mipLevels > 0)
        {
            header->dwFlags |= DDS_HEADER_FLAGS_MIPMAP;

            if (metadata.mipLevels > UINT32_MAX)
                return E_INVALIDARG;

            header->dwMipMapCount = static_cast<uint32_t>(metadata.mipLevels);

            if (header->dwMipMapCount > 1)
                header->dwCaps |= DDS_SURFACE_FLAGS_MIPMAP;
        }

        switch (metadata.dimension)
        {
        case TEX_DIMENSION_TEXTURE1D:
            if (metadata.width > UINT32_MAX)
                return E_INVALIDARG;

            header->dwWidth = static_cast<uint32_t>(metadata.width);
            header->dwHeight = header->dwDepth = 1;
            break;

        case TEX_DIMENSION_TEXTURE2D:
            if (metadata.height > UINT32_MAX
                || metadata.width > UINT32_MAX)
                return E_INVALIDARG;

            header->dwHeight = static_cast<uint32_t>(metadata.height);
            header->dwWidth = static_cast<uint32_t>(metadata.width);
            header->dwDepth = 1;

            if (metadata.IsCubemap())
            {
                header->dwCaps |= DDS_SURFACE_FLAGS_CUBEMAP;
                header->dwCaps2 |= DDS_CUBEMAP_ALLFACES;
            }
            break;

        case TEX_DIMENSION_TEXTURE3D:
            if (metadata.height > UINT32_MAX
                || metadata.width > UINT32_MAX
                || metadata.depth > UINT32_MAX)
                return E_INVALIDARG;

            header->dwFlags |= DDS_HEADER_FLAGS_VOLUME;
            header->dwCaps2 |= DDS_FLAGS_VOLUME;
            header->dwHeight = static_cast<uint32_t>(metadata.height);
            header->dwWidth = static_cast<uint32_t>(metadata.width);
            header->dwDepth = static_cast<uint32_t>(metadata.depth);
            break;

        default:
            return E_FAIL;
        }

        size_t rowPitch, slicePitch;
        ComputePitch(metadata.format, metadata.width, metadata.height, rowPitch, slicePitch, CP_FLAGS_NONE);

        if (slicePitch > UINT32_MAX
            || rowPitch > UINT32_MAX)
            return E_FAIL;

        if (IsCompressed(metadata.format))
        {
            header->dwFlags |= DDS_HEADER_FLAGS_LINEARSIZE;
            header->dwPitchOrLinearSize = static_cast<uint32_t>(slicePitch);
        }
        else
        {
            header->dwFlags |= DDS_HEADER_FLAGS_PITCH;
            header->dwPitchOrLinearSize = static_cast<uint32_t>(rowPitch);
        }

        memcpy_s(&header->ddspf, sizeof(header->ddspf), &DDSPF_XBOX, sizeof(DDS_PIXELFORMAT));

        // Setup XBOX extended header
        auto xboxext = reinterpret_cast<DDS_HEADER_XBOX*>(reinterpret_cast<uint8_t*>(header) + sizeof(DDS_HEADER));

        memset(xboxext, 0, sizeof(DDS_HEADER_XBOX));
        xboxext->dxgiFormat = metadata.format;
        xboxext->resourceDimension = metadata.dimension;

        if (metadata.arraySize > UINT32_MAX)
            return E_INVALIDARG;

        static_assert(TEX_MISC_TEXTURECUBE == DDS_RESOURCE_MISC_TEXTURECUBE, "DDS header mismatch");
        xboxext->miscFlag = metadata.miscFlags & ~TEX_MISC_TEXTURECUBE;

        if (metadata.miscFlags & TEX_MISC_TEXTURECUBE)
        {
            xboxext->miscFlag |= TEX_MISC_TEXTURECUBE;
            assert((metadata.arraySize % 6) == 0);
            xboxext->arraySize = static_cast<UINT>(metadata.arraySize / 6);
        }
        else
        {
            xboxext->arraySize = static_cast<UINT>(metadata.arraySize);
        }

        static_assert(TEX_MISC2_ALPHA_MODE_MASK == DDS_MISC_FLAGS2_ALPHA_MODE_MASK, "DDS header mismatch");

        static_assert(TEX_ALPHA_MODE_UNKNOWN == DDS_ALPHA_MODE_UNKNOWN, "DDS header mismatch");
        static_assert(TEX_ALPHA_MODE_STRAIGHT == DDS_ALPHA_MODE_STRAIGHT, "DDS header mismatch");
        static_assert(TEX_ALPHA_MODE_PREMULTIPLIED == DDS_ALPHA_MODE_PREMULTIPLIED, "DDS header mismatch");
        static_assert(TEX_ALPHA_MODE_OPAQUE == DDS_ALPHA_MODE_OPAQUE, "DDS header mismatch");
        static_assert(TEX_ALPHA_MODE_CUSTOM == DDS_ALPHA_MODE_CUSTOM, "DDS header mismatch");

        xboxext->miscFlags2 = metadata.miscFlags2;

        xboxext->tileMode = xbox.GetTileMode();
        xboxext->baseAlignment = xbox.GetAlignment();
        xboxext->dataSize = xbox.GetSize();
        xboxext->xdkVer = _XDK_VER;

        return S_OK;
    }
}


//=====================================================================================
// Entry-points
//=====================================================================================

//-------------------------------------------------------------------------------------
// Obtain metadata from DDS file in memory/on disk
//-------------------------------------------------------------------------------------

_Use_decl_annotations_
HRESULT Xbox::GetMetadataFromDDSMemory(
    const void* pSource,
    size_t size,
    TexMetadata& metadata,
    bool& isXbox)
{
    if (!pSource || !size)
        return E_INVALIDARG;

    isXbox = false;

    HRESULT hr = DecodeDDSHeader(pSource, size, metadata, nullptr, nullptr, nullptr);

    if (hr == S_FALSE)
    {
        hr = DirectX::GetMetadataFromDDSMemory(pSource, size, DirectX::DDS_FLAGS_NONE, metadata);
    }
    else if (SUCCEEDED(hr))
    {
        isXbox = true;
    }

    return hr;
}

_Use_decl_annotations_
HRESULT Xbox::GetMetadataFromDDSFile(
    const wchar_t* szFile,
    TexMetadata& metadata,
    bool& isXbox)
{
    if (!szFile)
        return E_INVALIDARG;

    isXbox = false;

#if (_WIN32_WINNT >= _WIN32_WINNT_WIN8)
    ScopedHandle hFile(safe_handle(CreateFile2(szFile, GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING, 0)));
#else
    ScopedHandle hFile(safe_handle(CreateFileW(szFile, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING,
        FILE_FLAG_SEQUENTIAL_SCAN, 0)));
#endif
    if (!hFile)
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    // Get the file size
    FILE_STANDARD_INFO fileInfo;
    if (!GetFileInformationByHandleEx(hFile.get(), FileStandardInfo, &fileInfo, sizeof(fileInfo)))
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    // File is too big for 32-bit allocation, so reject read (4 GB should be plenty large enough for a valid DDS file)
    if (fileInfo.EndOfFile.HighPart > 0)
    {
        return HRESULT_FROM_WIN32(ERROR_FILE_TOO_LARGE);
    }

    // Need at least enough data to fill the standard header and magic number to be a valid DDS
    if (fileInfo.EndOfFile.LowPart < (sizeof(DDS_HEADER) + sizeof(uint32_t)))
    {
        return E_FAIL;
    }

    // Read the header in (including extended header if present)
    uint8_t header[XBOX_HEADER_SIZE];

    DWORD bytesRead = 0;
    if (!ReadFile(hFile.get(), header, XBOX_HEADER_SIZE, &bytesRead, 0))
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    HRESULT hr = DecodeDDSHeader(header, bytesRead, metadata, nullptr, nullptr, nullptr);

    if (hr == S_FALSE)
    {
        hr = DirectX::GetMetadataFromDDSMemory(header, bytesRead, DirectX::DDS_FLAGS_NONE, metadata);
    }
    else if (SUCCEEDED(hr))
    {
        isXbox = true;
    }

    return hr;
}


//-------------------------------------------------------------------------------------
// Load a DDS file in memory
//-------------------------------------------------------------------------------------
_Use_decl_annotations_
HRESULT Xbox::LoadFromDDSMemory(
    const void* pSource,
    size_t size,
    TexMetadata* metadata,
    XboxImage& xbox)
{
    if (!pSource || !size)
        return E_INVALIDARG;

    xbox.Release();

    TexMetadata mdata;
    uint32_t dataSize;
    uint32_t baseAlignment;
    XG_TILE_MODE tmode;
    HRESULT hr = DecodeDDSHeader(pSource, size, mdata, &tmode, &dataSize, &baseAlignment);
    if (hr == S_FALSE)
    {
        // It's a DDS, but not an XBOX variant
        return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
    }
    if (FAILED(hr))
    {
        return hr;
    }

    if (!dataSize || !baseAlignment)
    {
        return E_FAIL;
    }

    if (size <= XBOX_HEADER_SIZE)
    {
        return E_FAIL;
    }

    // Copy tiled data
    size_t remaining = size - XBOX_HEADER_SIZE;

    if (remaining < dataSize)
    {
        return HRESULT_FROM_WIN32(ERROR_HANDLE_EOF);
    }

    hr = xbox.Initialize(mdata, tmode, dataSize, baseAlignment);
    if (FAILED(hr))
        return hr;

    assert(xbox.GetPointer() != 0);

    memcpy(xbox.GetPointer(), reinterpret_cast<const uint8_t*>(pSource) + XBOX_HEADER_SIZE, dataSize);

    if (metadata)
        memcpy(metadata, &mdata, sizeof(TexMetadata));

    return S_OK;
}


//-------------------------------------------------------------------------------------
// Load a DDS file from disk
//-------------------------------------------------------------------------------------
_Use_decl_annotations_
HRESULT Xbox::LoadFromDDSFile(
    const wchar_t* szFile,
    TexMetadata* metadata,
    XboxImage& xbox)
{
    if (!szFile)
        return E_INVALIDARG;

    xbox.Release();

#if (_WIN32_WINNT >= _WIN32_WINNT_WIN8)
    ScopedHandle hFile(safe_handle(CreateFile2(szFile, GENERIC_READ, FILE_SHARE_READ, OPEN_EXISTING, 0)));
#else
    ScopedHandle hFile(safe_handle(CreateFileW(szFile, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING,
        FILE_FLAG_SEQUENTIAL_SCAN, 0)));
#endif

    if (!hFile)
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    // Get the file size
    FILE_STANDARD_INFO fileInfo;
    if (!GetFileInformationByHandleEx(hFile.get(), FileStandardInfo, &fileInfo, sizeof(fileInfo)))
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    // File is too big for 32-bit allocation, so reject read (4 GB should be plenty large enough for a valid DDS file)
    if (fileInfo.EndOfFile.HighPart > 0)
    {
        return HRESULT_FROM_WIN32(ERROR_FILE_TOO_LARGE);
    }

    // Need at least enough data to fill the standard header and magic number to be a valid DDS
    if (fileInfo.EndOfFile.LowPart < (sizeof(DDS_HEADER) + sizeof(uint32_t)))
    {
        return E_FAIL;
    }

    // Read the header in (including extended header if present)
    uint8_t header[XBOX_HEADER_SIZE];

    DWORD bytesRead = 0;
    if (!ReadFile(hFile.get(), header, XBOX_HEADER_SIZE, &bytesRead, 0))
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    TexMetadata mdata;
    XG_TILE_MODE tmode;
    uint32_t dataSize;
    uint32_t baseAlignment;
    HRESULT hr = DecodeDDSHeader(header, bytesRead, mdata, &tmode, &dataSize, &baseAlignment);
    if (hr == S_FALSE)
    {
        // It's a DDS, but not an XBOX variant
        return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
    }
    if (FAILED(hr))
        return hr;

    if (!dataSize || !baseAlignment)
    {
        return E_FAIL;
    }

    // Read tiled data
    DWORD remaining = fileInfo.EndOfFile.LowPart - XBOX_HEADER_SIZE;
    if (remaining == 0)
        return E_FAIL;

    if (remaining < dataSize)
    {
        return HRESULT_FROM_WIN32(ERROR_HANDLE_EOF);
    }

    hr = xbox.Initialize(mdata, tmode, dataSize, baseAlignment);
    if (FAILED(hr))
        return hr;

    assert(xbox.GetPointer() != 0);

    if (!ReadFile(hFile.get(), xbox.GetPointer(), dataSize, &bytesRead, 0))
    {
        xbox.Release();
        return HRESULT_FROM_WIN32(GetLastError());
    }

    if (metadata)
        memcpy(metadata, &mdata, sizeof(TexMetadata));

    return S_OK;
}


//-------------------------------------------------------------------------------------
// Save a DDS file to memory
//-------------------------------------------------------------------------------------
_Use_decl_annotations_
HRESULT Xbox::SaveToDDSMemory(const XboxImage& xbox, Blob& blob)
{
    if (!xbox.GetPointer() || !xbox.GetSize() || !xbox.GetAlignment())
        return E_INVALIDARG;

    blob.Release();

    HRESULT hr = blob.Initialize(XBOX_HEADER_SIZE + xbox.GetSize());
    if (FAILED(hr))
        return hr;

    // Copy header
    auto pDestination = reinterpret_cast<uint8_t*>(blob.GetBufferPointer());
    assert(pDestination);

    hr = EncodeDDSHeader(xbox, pDestination, XBOX_HEADER_SIZE);
    if (FAILED(hr))
    {
        blob.Release();
        return hr;
    }

    // Copy tiled data
    size_t remaining = blob.GetBufferSize() - XBOX_HEADER_SIZE;
    pDestination += XBOX_HEADER_SIZE;

    if (!remaining)
    {
        blob.Release();
        return E_FAIL;
    }

    if (remaining < xbox.GetSize())
    {
        blob.Release();
        return E_UNEXPECTED;
    }

    memcpy(pDestination, xbox.GetPointer(), xbox.GetSize());

    return S_OK;
}


//-------------------------------------------------------------------------------------
// Save a DDS file to disk
//-------------------------------------------------------------------------------------
_Use_decl_annotations_
HRESULT Xbox::SaveToDDSFile(const XboxImage& xbox, const wchar_t* szFile)
{
    if (!szFile || !xbox.GetPointer() || !xbox.GetSize() || !xbox.GetAlignment())
        return E_INVALIDARG;

    // Create DDS Header
    uint8_t header[XBOX_HEADER_SIZE];
    HRESULT hr = EncodeDDSHeader(xbox, header, XBOX_HEADER_SIZE);
    if (FAILED(hr))
        return hr;

    // Create file and write header
#if (_WIN32_WINNT >= _WIN32_WINNT_WIN8)
    ScopedHandle hFile(safe_handle(CreateFile2(szFile, GENERIC_WRITE, 0, CREATE_ALWAYS, 0)));
#else
    ScopedHandle hFile(safe_handle(CreateFileW(szFile, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0)));
#endif
    if (!hFile)
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    DWORD bytesWritten;
    if (!WriteFile(hFile.get(), header, static_cast<DWORD>(XBOX_HEADER_SIZE), &bytesWritten, 0))
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    if (bytesWritten != XBOX_HEADER_SIZE)
    {
        return E_FAIL;
    }

    // Write tiled data
    if (!WriteFile(hFile.get(), xbox.GetPointer(), static_cast<DWORD>(xbox.GetSize()), &bytesWritten, 0))
    {
        return HRESULT_FROM_WIN32(GetLastError());
    }

    if (bytesWritten != xbox.GetSize())
    {
        return E_FAIL;
    }

    return S_OK;
}
