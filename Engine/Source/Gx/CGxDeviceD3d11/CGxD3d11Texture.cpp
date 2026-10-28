#include <cstdint>
#include <cstring>
#include "Engine/Source/Gx/CGxDeviceD3d11/CGxDeviceD3d11.h"
#include "Engine/Source/Gx/Blit.h"
#include "Engine/Source/Gx/CGxTex.h"
#include "Engine/Source/Gx/Gx.h"
#include "Engine/Source/Storm/H/storm.h"

const DXGI_FORMAT CGxDeviceD3d11::s_GxTexFormatToDxgiFormat[GxTexFormats_Last] = { DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_FORMAT_B4G4R4A4_UNORM, DXGI_FORMAT_B5G5R5A1_UNORM, DXGI_FORMAT_B5G6R5_UNORM, DXGI_FORMAT_BC1_UNORM, DXGI_FORMAT_BC2_UNORM, DXGI_FORMAT_BC3_UNORM, DXGI_FORMAT_R8G8_SNORM, DXGI_FORMAT_R16G16_FLOAT, DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_R24G8_TYPELESS };

const DXGI_FORMAT CGxDeviceD3d11::s_GxTexFormatToSrvFormat[GxTexFormats_Last] = { DXGI_FORMAT_UNKNOWN, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_FORMAT_B4G4R4A4_UNORM, DXGI_FORMAT_B5G5R5A1_UNORM, DXGI_FORMAT_B5G6R5_UNORM, DXGI_FORMAT_BC1_UNORM, DXGI_FORMAT_BC2_UNORM, DXGI_FORMAT_BC3_UNORM, DXGI_FORMAT_R8G8_SNORM, DXGI_FORMAT_R16G16_FLOAT, DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_R24_UNORM_X8_TYPELESS };

CGxDeviceD3d11::TexData* CGxDeviceD3d11::ITexGetData(CGxTex* texId) {
    return static_cast<TexData*>(texId->m_apiSpecificData);
}

int32_t CGxDeviceD3d11::ITexAllocD3dTexture(CGxTex* texId) {
    uint32_t width;
    uint32_t height;
    uint32_t baseMip;
    uint32_t mipCount;
    this->ITexWHDStartEnd(texId, width, height, baseMip, mipCount);

    bool cube = texId->m_target == GxTex_CubeMap;
    bool depth = texId->m_format == GxTex_D24X8;
    uint32_t levels = mipCount - baseMip;

    D3D11_TEXTURE2D_DESC desc;
    memset(&desc, 0, sizeof(desc));
    desc.Width = width;
    desc.Height = cube ? width : height;
    desc.MipLevels = levels;
    desc.ArraySize = cube ? GxTexCubeFaces_Last : 1;
    desc.Format = s_GxTexFormatToDxgiFormat[texId->m_format];
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    desc.MiscFlags = cube ? D3D11_RESOURCE_MISC_TEXTURECUBE : 0;

    if (texId->m_flags.m_renderTarget) {
        desc.BindFlags |= depth ? D3D11_BIND_DEPTH_STENCIL : D3D11_BIND_RENDER_TARGET;
    }

    if (desc.Format == DXGI_FORMAT_UNKNOWN) {
        return 0;
    }

    TexData* data = static_cast<TexData*>(SMemAlloc(sizeof(TexData), __FILE__, __LINE__, 0));
    memset(data, 0, sizeof(TexData));
    data->m_levels = levels;

    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
    memset(&srvDesc, 0, sizeof(srvDesc));
    srvDesc.Format = s_GxTexFormatToSrvFormat[texId->m_format];

    if (cube) {
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
        srvDesc.TextureCube.MipLevels = levels;
    } else {
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MipLevels = levels;
    }

    if (FAILED(this->m_d3dDevice->CreateTexture2D(&desc, nullptr, &data->m_texture)) || FAILED(this->m_d3dDevice->CreateShaderResourceView(data->m_texture, &srvDesc, &data->m_srv))) {
        if (data->m_texture) {
            data->m_texture->Release();
        }

        SMemFree(data, __FILE__, __LINE__, 0);
        return 0;
    }

    texId->m_apiSpecificData = data;

    return 1;
}

void CGxDeviceD3d11::ITexAcquire(CGxTex* texId) {
    if (texId->m_apiSpecificData) {
        return;
    }

    texId->m_lastUsedFrame = this->m_frameCount;
    this->ITexAllocD3dTexture(texId);
}

void CGxDeviceD3d11::ITexReleaseD3dTexture(CGxTex* texId) {
    TexData* data = this->ITexGetData(texId);

    if (!data) {
        return;
    }

    for (uint32_t i = 0; i < GxTexCubeFaces_Last; i++) {
        if (data->m_rtv[i]) {
            data->m_rtv[i]->Release();
        }
    }

    if (data->m_dsv) {
        data->m_dsv->Release();
    }

    if (data->m_srv) {
        data->m_srv->Release();
    }

    if (data->m_texture) {
        data->m_texture->Release();
    }

    SMemFree(data, __FILE__, __LINE__, 0);
    texId->m_apiSpecificData = nullptr;
}

void CGxDeviceD3d11::ITexCreate(CGxTex* texId) {
    if (!texId->m_flags.m_renderTarget) {
        texId->m_apiSpecificData2 = SMemAlloc(this->ITexBitsSize(texId, 0xFFFFFFFF, 0xFFFFFFFF), __FILE__, __LINE__, 0);
        texId->m_needsCreation = 0;
    } else {
        this->ITexAcquire(texId);

        if (texId->m_apiSpecificData) {
            texId->m_needsCreation = 0;
        }
    }
}

void CGxDeviceD3d11::ITexMarkAsUpdated(CGxTex* texId) {
    if (!texId->m_needsUpdate || !this->m_context) {
        return;
    }

    if (texId->m_needsCreation || (!texId->m_apiSpecificData && !texId->m_apiSpecificData2)) {
        this->ITexCreate(texId);
    }

    if (!texId->m_needsCreation && (texId->m_apiSpecificData || texId->m_apiSpecificData2)) {
        if (texId->m_userFunc) {
            this->ITexUpload(texId);
        }

        CGxDevice::ITexMarkAsUpdated(texId);
    }
}

void CGxDeviceD3d11::ITexUploadRect(CGxTex* texId, uint32_t face, uint32_t level, const RECT& rect, const void* bits, uint32_t pitch) {
    TexData* data = this->ITexGetData(texId);

    if (!data || level >= data->m_levels) {
        return;
    }

    uint32_t width;
    uint32_t height;
    uint32_t baseMip;
    uint32_t mipCount;
    this->ITexWHDStartEnd(texId, width, height, baseMip, mipCount);

    uint32_t mipWidth = width >> level;
    uint32_t mipHeight = height >> level;
    mipWidth = mipWidth ? mipWidth : 1;
    mipHeight = mipHeight ? mipHeight : 1;

    D3D11_BOX box;
    box.left = rect.left;
    box.top = rect.top;
    box.front = 0;
    box.right = rect.right;
    box.bottom = rect.bottom;
    box.back = 1;

    bool whole = rect.left == 0 && rect.top == 0 && static_cast<uint32_t>(rect.right) >= mipWidth && static_cast<uint32_t>(rect.bottom) >= mipHeight;

    this->m_d3dContext->UpdateSubresource(data->m_texture, D3D11CalcSubresource(level, face, data->m_levels), whole ? nullptr : &box, bits, pitch, 0);
}

void CGxDeviceD3d11::ITexUploadAll(CGxTex* texId) {
    uint32_t width;
    uint32_t height;
    uint32_t baseMip;
    uint32_t mipCount;
    this->ITexWHDStartEnd(texId, width, height, baseMip, mipCount);

    uint32_t blockSize = s_texFormatBlockSize[texId->m_format];
    const uint8_t* bits = static_cast<const uint8_t*>(texId->m_apiSpecificData2);
    uint32_t faceCount = texId->m_target == GxTex_CubeMap ? GxTexCubeFaces_Last : 1;

    if (!bits || !texId->m_apiSpecificData) {
        return;
    }

    for (uint32_t face = 0; face < faceCount; face++) {
        uint32_t mipWidth = width;
        uint32_t mipHeight = height;
        uint32_t level = 0;

        for (uint32_t mip = baseMip; mip != mipCount; mip++, level++) {
            RECT rect;
            rect.left = 0;
            rect.top = 0;
            rect.right = mipWidth;
            rect.bottom = mipHeight;

            uint32_t pitch = GxCalcTexelStrideInBytes(texId->m_format, mipWidth);
            this->ITexUploadRect(texId, face, level, rect, bits, pitch);

            bits += pitch * ((mipHeight + blockSize - 1) / blockSize);

            mipHeight >>= 1;
            mipWidth >>= 1;

            if (!mipWidth) {
                mipWidth = 1;
            }

            if (!mipHeight) {
                mipHeight = 1;
            }
        }
    }
}

void CGxDeviceD3d11::ITexUpload(CGxTex* texId) {
    uint32_t texelStrideInBytes;
    const void* texels = nullptr;
    texId->m_userFunc(GxTex_Lock, texId->m_width, texId->m_height, 0, 0, texId->m_userArg, texelStrideInBytes, texels);

    uint32_t width;
    uint32_t height;
    uint32_t baseMip;
    uint32_t mipCount;
    this->ITexWHDStartEnd(texId, width, height, baseMip, mipCount);

    int32_t compressed = texId->m_dataFormat >= GxTex_Dxt1 && texId->m_dataFormat <= GxTex_Dxt5;
    uint32_t blockSize = s_texFormatBlockSize[texId->m_format];
    uint32_t faceCount = texId->m_target == GxTex_CubeMap ? GxTexCubeFaces_Last : 1;
    uint8_t* bits = static_cast<uint8_t*>(texId->m_apiSpecificData2);

    for (uint32_t face = 0; face < faceCount; face++) {
        uint32_t mipWidth = width;
        uint32_t mipHeight = height;
        uint32_t level = 0;

        for (uint32_t mip = baseMip; mip != mipCount; mip++, level++) {
            uint32_t pitch = GxCalcTexelStrideInBytes(texId->m_format, mipWidth);
            texels = nullptr;
            texId->m_userFunc(GxTex_Latch, mipWidth, mipHeight, face, mip, texId->m_userArg, texelStrideInBytes, texels);

            if (texels && !texId->m_flags.m_renderTarget) {
                RECT rect;
                rect.left = texId->m_updateRect.minX >> mip;
                rect.top = texId->m_updateRect.minY >> mip;
                rect.bottom = rect.top + 1 > texId->m_updateRect.maxY >> mip ? rect.top + 1 : texId->m_updateRect.maxY >> mip;
                rect.right = rect.left + 1 > texId->m_updateRect.maxX >> mip ? rect.left + 1 : texId->m_updateRect.maxX >> mip;

                if (compressed) {
                    rect.top &= ~3;
                    rect.left &= ~3;
                    rect.bottom = (rect.bottom + 3) & ~3;
                    rect.right = (rect.right + 3) & ~3;
                }

                rect.left = rect.left < 0 ? 0 : rect.left;
                rect.top = rect.top < 0 ? 0 : rect.top;
                rect.right = rect.right > static_cast<LONG>(mipWidth) ? mipWidth : rect.right;
                rect.bottom = rect.bottom > static_cast<LONG>(mipHeight) ? mipHeight : rect.bottom;

                if (rect.right <= rect.left || rect.bottom <= rect.top) {
                    if (bits) {
                        bits += pitch * ((mipHeight + blockSize - 1) / blockSize);
                    }

                    mipWidth = mipWidth > 1 ? mipWidth >> 1 : 1;
                    mipHeight = mipHeight > 1 ? mipHeight >> 1 : 1;
                    continue;
                }

                const uint8_t* src = static_cast<const uint8_t*>(texels);

                if (!texId->m_flags.m_bit15) {
                    EGxTexFormat dataFormat = texId->m_dataFormat;

                    if (compressed) {
                        src += s_texFormatBytesPerBlock[dataFormat] * (rect.left >> 2) + texelStrideInBytes * (rect.top >> 2);
                    } else {
                        src += ((rect.left * s_texFormatBitDepth[dataFormat]) >> 3) + texelStrideInBytes * rect.top;
                    }
                }

                uint8_t* dest = bits + s_texFormatBytesPerBlock[texId->m_format] * (rect.left / blockSize) + pitch * (rect.top / blockSize);

                C2iVector size;
                size.x = rect.right - rect.left;
                size.y = rect.bottom - rect.top;
                Blit(size, BlitAlpha_0, src, texelStrideInBytes, GxGetBlitFormat(texId->m_dataFormat), dest, pitch, GxGetBlitFormat(texId->m_format));

                if (texId->m_apiSpecificData) {
                    this->ITexUploadRect(texId, face, level, rect, dest, pitch);
                }
            }

            if (bits) {
                bits += pitch * ((mipHeight + blockSize - 1) / blockSize);
            }

            mipHeight >>= 1;
            mipWidth >>= 1;

            if (!mipWidth) {
                mipWidth = 1;
            }

            if (!mipHeight) {
                mipHeight = 1;
            }
        }
    }

    texels = nullptr;
    texId->m_userFunc(GxTex_Unlock, texId->m_width, texId->m_height, 0, 0, texId->m_userArg, texelStrideInBytes, texels);
}

void CGxDeviceD3d11::TexDestroy(CGxTex* texId) {
    if (texId->m_apiSpecificData2) {
        delete[] static_cast<uint8_t*>(texId->m_apiSpecificData2);
        texId->m_apiSpecificData2 = nullptr;
    }

    this->ITexReleaseD3dTexture(texId);
    CGxDevice::TexDestroy(texId);
}

void CGxDeviceD3d11::ITexForceRecreation(int32_t all) {
    for (CGxTex* texId = this->m_textures.Head(); texId; texId = this->m_textures.Next(texId)) {
        this->ITexReleaseD3dTexture(texId);

        if (all || texId->m_flags.m_renderTarget) {
            if (texId->m_apiSpecificData2) {
                delete[] static_cast<uint8_t*>(texId->m_apiSpecificData2);
                texId->m_apiSpecificData2 = nullptr;
            }

            CiRect updateRect = {};

            texId->m_needsCreation = 1;
            this->TexMarkForUpdate(texId, updateRect, 0);

            uint32_t texelStrideInBytes;
            const void* texels;
            texId->m_userFunc(GxTex_Recreate, texId->m_width, texId->m_height, 0, 0, texId->m_userArg, texelStrideInBytes, texels);
        }
    }

    this->IUnbindTextures();

    if (this->m_hwnd && this->m_d3dDevice) {
        this->NotifyOnTextureRecreation();
    }
}

int32_t CGxDeviceD3d11::IEnsureTexture(CGxTex* texId) {
    bool acquired = false;

    if (!texId->m_flags.m_renderTarget && !texId->m_apiSpecificData) {
        this->ITexAcquire(texId);
        acquired = true;
    }

    this->ITexMarkAsUpdated(texId);

    if (acquired) {
        this->ITexUploadAll(texId);
    }

    TexData* data = this->ITexGetData(texId);

    return data && data->m_srv;
}

ID3D11SamplerState* CGxDeviceD3d11::IGetSamplerState(const CGxTexFlags& flags) {
    uint32_t anisotropy = flags.m_maxAnisotropy ? flags.m_maxAnisotropy : 1;
    uint32_t key = flags.m_filter | (flags.m_wrapU << 3) | (flags.m_wrapV << 4) | (anisotropy << 5);
    ID3D11SamplerState*& state = this->m_samplerStates[key];

    if (state) {
        return state;
    }

    D3D11_SAMPLER_DESC desc;
    memset(&desc, 0, sizeof(desc));
    desc.Filter = s_filterModes[flags.m_filter < GxTexFilters_Last ? flags.m_filter : GxTex_Linear];
    desc.AddressU = flags.m_wrapU ? D3D11_TEXTURE_ADDRESS_WRAP : D3D11_TEXTURE_ADDRESS_CLAMP;
    desc.AddressV = flags.m_wrapV ? D3D11_TEXTURE_ADDRESS_WRAP : D3D11_TEXTURE_ADDRESS_CLAMP;
    desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    desc.MaxAnisotropy = anisotropy > 16 ? 16 : anisotropy;
    desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    desc.MaxLOD = D3D11_FLOAT32_MAX;

    this->m_d3dDevice->CreateSamplerState(&desc, &state);

    return state;
}

// DX9 depth textures compare on sampling. A nearest filter gives one comparison, linear gives hardware PCF.
ID3D11SamplerState* CGxDeviceD3d11::IGetShadowSamplerState(const CGxTexFlags& flags) {
    uint32_t index = flags.m_filter == GxTex_Nearest ? 0 : 1;
    ID3D11SamplerState*& state = this->m_shadowSamplers[index];

    if (state) {
        return state;
    }

    D3D11_SAMPLER_DESC desc;
    memset(&desc, 0, sizeof(desc));
    desc.Filter = index ? D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT : D3D11_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
    desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    desc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    desc.MaxAnisotropy = 1;
    desc.ComparisonFunc = D3D11_COMPARISON_LESS_EQUAL;
    desc.MaxLOD = D3D11_FLOAT32_MAX;

    this->m_d3dDevice->CreateSamplerState(&desc, &state);

    return state;
}

void CGxDeviceD3d11::ISetTexture(uint32_t slot, CGxTex* texId) {
    ID3D11ShaderResourceView* srv = nullptr;
    ID3D11SamplerState* sampler = nullptr;

    if (texId && this->IEnsureTexture(texId)) {
        srv = this->ITexGetData(texId)->m_srv;
        sampler = texId->m_format == GxTex_D24X8 ? this->IGetShadowSamplerState(texId->m_flags) : this->IGetSamplerState(texId->m_flags);
    }

    if (srv != this->m_boundSrv[slot]) {
        this->m_d3dContext->PSSetShaderResources(slot, 1, &srv);
        this->m_boundSrv[slot] = srv;
    }

    if (sampler != this->m_boundSampler[slot]) {
        this->m_d3dContext->PSSetSamplers(slot, 1, &sampler);
        this->m_boundSampler[slot] = sampler;
    }
}

void CGxDeviceD3d11::DeviceSetRenderTarget(EGxBuffer buffer, CGxTex* texture, uint32_t plane) {
    if (!this->m_context) {
        return;
    }

    CGxDevice::TextureTarget& target = this->m_textureTarget[buffer];

    if (target.m_texture == texture && target.m_plane == plane) {
        return;
    }

    CGxDevice::DeviceSetRenderTarget(buffer, texture, plane);

    if (target.m_apiSpecific) {
        static_cast<IUnknown*>(target.m_apiSpecific)->Release();
        target.m_apiSpecific = nullptr;
    }

    if (texture) {
        if (texture->m_needsCreation || (!texture->m_apiSpecificData && !texture->m_apiSpecificData2)) {
            this->ITexCreate(texture);
        }

        TexData* data = this->ITexGetData(texture);

        if (data) {
            if (buffer == GxBuffers_Color) {
                uint32_t face = texture->m_target == GxTex_CubeMap ? plane : 0;

                if (!data->m_rtv[face]) {
                    D3D11_RENDER_TARGET_VIEW_DESC desc;
                    memset(&desc, 0, sizeof(desc));
                    desc.Format = s_GxTexFormatToDxgiFormat[texture->m_format];

                    if (texture->m_target == GxTex_CubeMap) {
                        desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2DARRAY;
                        desc.Texture2DArray.FirstArraySlice = face;
                        desc.Texture2DArray.ArraySize = 1;
                    } else {
                        desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
                    }

                    this->m_d3dDevice->CreateRenderTargetView(data->m_texture, &desc, &data->m_rtv[face]);
                }

                if (data->m_rtv[face]) {
                    data->m_rtv[face]->AddRef();
                    target.m_apiSpecific = data->m_rtv[face];
                }
            } else {
                if (!data->m_dsv) {
                    D3D11_DEPTH_STENCIL_VIEW_DESC desc;
                    memset(&desc, 0, sizeof(desc));
                    desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
                    desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
                    this->m_d3dDevice->CreateDepthStencilView(data->m_texture, &desc, &data->m_dsv);
                }

                if (data->m_dsv) {
                    data->m_dsv->AddRef();
                    target.m_apiSpecific = data->m_dsv;
                }
            }
        }
    }

    ID3D11RenderTargetView* rtv = this->m_backBufferRtv;
    ID3D11DepthStencilView* dsv = this->m_depthDsv;
    CGxDevice::TextureTarget& colorTarget = this->m_textureTarget[GxBuffers_Color];
    CGxDevice::TextureTarget& depthTarget = this->m_textureTarget[GxBuffers_Depth];

    if (colorTarget.m_apiSpecific) {
        rtv = static_cast<ID3D11RenderTargetView*>(colorTarget.m_apiSpecific);

        if (!depthTarget.m_apiSpecific) {
            this->ICreateScratchDepth(colorTarget.m_texture->m_width, colorTarget.m_texture->m_height);
            dsv = this->m_scratchDepthDsv;
        }
    }

    if (depthTarget.m_apiSpecific) {
        dsv = static_cast<ID3D11DepthStencilView*>(depthTarget.m_apiSpecific);
    }

    this->m_curRtv = rtv;
    this->m_curDsv = dsv;
    this->ISetRenderTargets();

    GxXformSetViewport(this->m_viewport.m_x.m_l, this->m_viewport.m_x.m_h, this->m_viewport.m_y.m_l, this->m_viewport.m_y.m_h, this->m_viewport.m_z.m_l, this->m_viewport.m_z.m_h);

    this->m_viewportDirty = 1;
    this->m_scissorRectDirty = 1;
}

void CGxDeviceD3d11::ICreateScratchDepth(uint32_t width, uint32_t height) {
    if (this->m_scratchDepthTexture && this->m_scratchDepthWidth >= width && this->m_scratchDepthHeight >= height) {
        return;
    }

    if (this->m_scratchDepthDsv) {
        this->m_scratchDepthDsv->Release();
        this->m_scratchDepthDsv = nullptr;
    }

    if (this->m_scratchDepthTexture) {
        this->m_scratchDepthTexture->Release();
        this->m_scratchDepthTexture = nullptr;
    }

    D3D11_TEXTURE2D_DESC desc;
    memset(&desc, 0, sizeof(desc));
    desc.Width = width;
    desc.Height = height;
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

    if (SUCCEEDED(this->m_d3dDevice->CreateTexture2D(&desc, nullptr, &this->m_scratchDepthTexture))) {
        this->m_d3dDevice->CreateDepthStencilView(this->m_scratchDepthTexture, nullptr, &this->m_scratchDepthDsv);
        this->m_scratchDepthWidth = width;
        this->m_scratchDepthHeight = height;
    }
}

void CGxDeviceD3d11::DeviceResolveDepthBuffer(CGxTex* texture) {
    if (!this->m_context || !texture) {
        return;
    }

    if (texture->m_needsCreation || (!texture->m_apiSpecificData && !texture->m_apiSpecificData2)) {
        this->ITexCreate(texture);
    }

    TexData* data = this->ITexGetData(texture);

    if (!data) {
        return;
    }

    if (!data->m_dsv) {
        D3D11_DEPTH_STENCIL_VIEW_DESC desc;
        memset(&desc, 0, sizeof(desc));
        desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
        this->m_d3dDevice->CreateDepthStencilView(data->m_texture, &desc, &data->m_dsv);
    }

    if (data->m_dsv) {
        this->m_curDsv = data->m_dsv;
        this->ISetRenderTargets();
    }
}

void CGxDeviceD3d11::DeviceCopyTex(CGxTex* source, uint32_t sourcePlane, CGxTex* dest, uint32_t destPlane) {
    if (!this->m_context || !source || !dest) {
        return;
    }

    if (source->m_needsCreation || (!source->m_apiSpecificData && !source->m_apiSpecificData2)) {
        this->ITexCreate(source);
    }

    if (dest->m_needsCreation || (!dest->m_apiSpecificData && !dest->m_apiSpecificData2)) {
        this->ITexCreate(dest);
    }

    TexData* sourceData = this->ITexGetData(source);
    TexData* destData = this->ITexGetData(dest);

    if (!sourceData || !destData || source->m_format != dest->m_format) {
        return;
    }

    uint32_t width = source->m_width < dest->m_width ? source->m_width : dest->m_width;
    uint32_t height = source->m_height < dest->m_height ? source->m_height : dest->m_height;

    D3D11_BOX box;
    box.left = 0;
    box.top = 0;
    box.front = 0;
    box.right = width;
    box.bottom = height;
    box.back = 1;

    uint32_t sourceFace = source->m_target == GxTex_CubeMap ? sourcePlane : 0;
    uint32_t destFace = dest->m_target == GxTex_CubeMap ? destPlane : 0;

    this->m_d3dContext->CopySubresourceRegion(destData->m_texture, D3D11CalcSubresource(0, destFace, destData->m_levels), 0, 0, 0, sourceData->m_texture, D3D11CalcSubresource(0, sourceFace, sourceData->m_levels), &box);
}

int32_t CGxDeviceD3d11::ICopyBackBuffer(CGxTex* dest, const CiRect& sourceRect, const C2iVector& destPos, uint32_t level, uint32_t face) {
    if (!this->m_context || this->m_sampleCount > 1) {
        return 0;
    }

    if (dest->m_needsCreation || (!dest->m_apiSpecificData && !dest->m_apiSpecificData2)) {
        this->ITexCreate(dest);
    }

    TexData* data = this->ITexGetData(dest);

    if (!data || level >= data->m_levels || s_GxTexFormatToDxgiFormat[dest->m_format] != this->m_backBufferFormat) {
        return 0;
    }

    ID3D11Texture2D* backBuffer = nullptr;

    if (FAILED(this->m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer)))) {
        return 0;
    }

    D3D11_BOX box;
    box.left = sourceRect.minX < 0 ? 0 : sourceRect.minX;
    box.top = sourceRect.minY < 0 ? 0 : sourceRect.minY;
    box.front = 0;
    box.right = static_cast<uint32_t>(sourceRect.maxX) > this->m_backBufferWidth ? this->m_backBufferWidth : sourceRect.maxX;
    box.bottom = static_cast<uint32_t>(sourceRect.maxY) > this->m_backBufferHeight ? this->m_backBufferHeight : sourceRect.maxY;
    box.back = 1;

    if (box.right > box.left && box.bottom > box.top) {
        this->m_d3dContext->CopySubresourceRegion(data->m_texture, D3D11CalcSubresource(level, face, data->m_levels), destPos.x, destPos.y, 0, backBuffer, 0, &box);
    }

    backBuffer->Release();

    return 1;
}

int32_t CGxDeviceD3d11::TexCopy(CGxTex* dest, const C2iVector& destPos, const C2iVector& sourcePos, const C2iVector& size, uint32_t level, uint32_t plane) {
    CiRect sourceRect;
    sourceRect.minX = sourcePos.x;
    sourceRect.minY = sourcePos.y;
    sourceRect.maxX = sourcePos.x + size.x;
    sourceRect.maxY = sourcePos.y + size.y;

    return this->ICopyBackBuffer(dest, sourceRect, destPos, level, plane);
}

int32_t CGxDeviceD3d11::TexStretch(CGxTex* dest, const CiRect* destRect, const CiRect* sourceRect, uint32_t level, uint32_t face) {
    CiRect source;
    source.minX = 0;
    source.minY = 0;
    source.maxX = this->m_backBufferWidth;
    source.maxY = this->m_backBufferHeight;

    if (sourceRect) {
        source = *sourceRect;
    }

    C2iVector destPos;
    destPos.x = destRect ? destRect->minX : 0;
    destPos.y = destRect ? destRect->minY : 0;

    // no scaling, a real stretch needs a draw
    if (destRect && (destRect->maxX - destRect->minX != source.maxX - source.minX || destRect->maxY - destRect->minY != source.maxY - source.minY)) {
        return 0;
    }

    return this->ICopyBackBuffer(dest, source, destPos, level, face);
}
