#include <cmath>
#include <cstdint>
#include <cstring>
#include "Engine/Source/Gx/CGxDeviceD3d11/CGxDeviceD3d11.h"
#include "Engine/Source/Gx/CGxBuf.h"
#include "Engine/Source/Gx/CGxPool.h"
#include "Engine/Source/Gx/Gx.h"
#include "Engine/Source/Storm/H/storm.h"

const DXGI_FORMAT CGxDeviceD3d11::s_gxAttribToDxgiFormat[7] = { DXGI_FORMAT_B8G8R8A8_UNORM, DXGI_FORMAT_R8G8B8A8_UINT, DXGI_FORMAT_R8G8B8A8_UNORM, DXGI_FORMAT_R32G32_FLOAT, DXGI_FORMAT_R32G32B32_FLOAT, DXGI_FORMAT_R16G16_SINT, DXGI_FORMAT_R32_FLOAT };

const uint32_t CGxDeviceD3d11::s_gxAttribToSize[7] = { 4, 4, 4, 8, 12, 4, 4 };

// OFFSET-15595: 0x0086D690
const char* const CGxDeviceD3d11::s_gxAttribToSemanticName[GxVAs_Last] = { "POSITION", "BLENDWEIGHT", "BLENDINDICES", "NORMAL", "COLOR", "COLOR", "TEXCOORD", "TEXCOORD", "TEXCOORD", "TEXCOORD", "TEXCOORD", "TEXCOORD", "TEXCOORD", "TEXCOORD" };

const uint32_t CGxDeviceD3d11::s_gxAttribToSemanticIndex[GxVAs_Last] = { 0, 0, 0, 0, 0, 1, 0, 1, 2, 3, 4, 5, 6, 7 };

// the alpha channel can't use color factors, so the *Alpha tables swap them for the alpha ones
const D3D11_BLEND CGxDeviceD3d11::s_srcBlend[12] = { D3D11_BLEND_ONE, D3D11_BLEND_ONE, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_DEST_COLOR, D3D11_BLEND_DEST_COLOR, D3D11_BLEND_DEST_COLOR, D3D11_BLEND_INV_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_ONE, D3D11_BLEND_BLEND_FACTOR };

const D3D11_BLEND CGxDeviceD3d11::s_srcBlendAlpha[12] = { D3D11_BLEND_ONE, D3D11_BLEND_ONE, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_DEST_ALPHA, D3D11_BLEND_DEST_ALPHA, D3D11_BLEND_DEST_ALPHA, D3D11_BLEND_INV_SRC_ALPHA, D3D11_BLEND_INV_SRC_ALPHA, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_ONE, D3D11_BLEND_BLEND_FACTOR };

const D3D11_BLEND CGxDeviceD3d11::s_dstBlend[12] = { D3D11_BLEND_ZERO, D3D11_BLEND_ZERO, D3D11_BLEND_INV_SRC_ALPHA, D3D11_BLEND_ONE, D3D11_BLEND_ZERO, D3D11_BLEND_SRC_COLOR, D3D11_BLEND_ONE, D3D11_BLEND_ONE, D3D11_BLEND_ZERO, D3D11_BLEND_ZERO, D3D11_BLEND_ONE, D3D11_BLEND_INV_BLEND_FACTOR };

const D3D11_BLEND CGxDeviceD3d11::s_dstBlendAlpha[12] = { D3D11_BLEND_ZERO, D3D11_BLEND_ZERO, D3D11_BLEND_INV_SRC_ALPHA, D3D11_BLEND_ONE, D3D11_BLEND_ZERO, D3D11_BLEND_SRC_ALPHA, D3D11_BLEND_ONE, D3D11_BLEND_ONE, D3D11_BLEND_ZERO, D3D11_BLEND_ZERO, D3D11_BLEND_ONE, D3D11_BLEND_INV_BLEND_FACTOR };

const D3D11_COMPARISON_FUNC CGxDeviceD3d11::s_cmpFunc[GxCmps_Last] = { D3D11_COMPARISON_LESS_EQUAL, D3D11_COMPARISON_EQUAL, D3D11_COMPARISON_GREATER_EQUAL, D3D11_COMPARISON_LESS };

const D3D11_FILTER CGxDeviceD3d11::s_filterModes[GxTexFilters_Last] = { D3D11_FILTER_MIN_MAG_MIP_POINT, D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT, D3D11_FILTER_MIN_MAG_MIP_POINT, D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT, D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_FILTER_ANISOTROPIC };

const D3D11_PRIMITIVE_TOPOLOGY CGxDeviceD3d11::s_primitiveTopology[GxPrims_Last] = { D3D11_PRIMITIVE_TOPOLOGY_POINTLIST, D3D11_PRIMITIVE_TOPOLOGY_LINELIST, D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST };

bool CGxDeviceD3d11::InputLayoutKey::operator==(const InputLayoutKey& key) const {
    return this->m_code == key.m_code && this->m_count == key.m_count && !memcmp(this->m_elements, key.m_elements, sizeof(uint32_t) * 2 * this->m_count);
}

size_t CGxDeviceD3d11::InputLayoutKeyHash::operator()(const InputLayoutKey& key) const {
    size_t hash = reinterpret_cast<size_t>(key.m_code) * 31 + key.m_count;

    for (uint32_t i = 0; i < key.m_count * 2; i++) {
        hash = hash * 16777619u ^ key.m_elements[i];
    }

    return hash;
}

void CGxDeviceD3d11::IReleaseStateObjects() {
    for (auto& entry : this->m_rasterizerStates) {
        entry.second->Release();
    }

    this->m_rasterizerStates.clear();

    for (auto& entry : this->m_blendStates) {
        entry.second->Release();
    }

    this->m_blendStates.clear();

    for (auto& entry : this->m_depthStates) {
        entry.second->Release();
    }

    this->m_depthStates.clear();

    for (auto& entry : this->m_inputLayouts) {
        entry.second->Release();
    }

    this->m_inputLayouts.clear();

    for (uint32_t i = 0; i < sizeof(this->m_samplerStates) / sizeof(this->m_samplerStates[0]); i++) {
        if (this->m_samplerStates[i]) {
            this->m_samplerStates[i]->Release();
            this->m_samplerStates[i] = nullptr;
        }
    }
}

void CGxDeviceD3d11::IRsSendToHw(EGxRenderState which) {
    switch (which) {
        case GxRs_BlendingMode:
        case GxRs_ColorWrite:
        case GxRs_BlendFactor:
            this->m_stateDirty |= StateDirty_Blend;
            break;

        case GxRs_DepthTest:
        case GxRs_DepthFunc:
        case GxRs_DepthWrite:
            this->m_stateDirty |= StateDirty_Depth;
            break;

        case GxRs_PolygonOffset:
        case GxRs_Culling:
        case GxRs_Multisample:
        case GxRs_ScissorTest:
            this->m_stateDirty |= StateDirty_Rasterizer;
            break;

        case GxRs_AlphaRef:
        case GxRs_FogStart:
        case GxRs_FogEnd:
        case GxRs_FogColor:
        case GxRs_Fog:
            this->m_stateDirty |= StateDirty_DeviceConstants;
            break;

        case GxRs_VertexShader:
            this->m_stateDirty |= StateDirty_InputLayout;
            break;

        default:
            break;
    }
}

void CGxDeviceD3d11::IStateSyncDeviceConstants() {
    if (!(this->m_stateDirty & StateDirty_DeviceConstants)) {
        return;
    }

    this->m_stateDirty &= ~StateDirty_DeviceConstants;

    DeviceConstants constants;
    memset(&constants, 0, sizeof(constants));

    int32_t alphaRef = this->m_appRenderStates[GxRs_AlphaRef].m_value.m_data.i[0];
    constants.m_alphaRef[0] = alphaRef - 0.5f;
    constants.m_alphaRef[1] = alphaRef > 0 ? 1.0f : 0.0f;

    float fogStart = this->m_appRenderStates[GxRs_FogStart].m_value.m_data.f[0];
    float fogEnd = this->m_appRenderStates[GxRs_FogEnd].m_value.m_data.f[0];
    bool fogEnable = (this->m_appMasterEnables & (1 << GxMasterEnable_Fog)) && this->m_appRenderStates[GxRs_Fog].m_value.m_data.i[0];
    constants.m_fog[0] = fogStart;
    constants.m_fog[1] = fogEnd;
    constants.m_fog[2] = fogEnable ? 1.0f : 0.0f;
    constants.m_fog[3] = fogEnd != fogStart ? 1.0f / (fogEnd - fogStart) : 0.0f;

    uint32_t fogColor = this->m_appRenderStates[GxRs_FogColor].m_value.m_data.u[0];
    constants.m_fogColor[0] = ((fogColor >> 16) & 0xFF) * oo255;
    constants.m_fogColor[1] = ((fogColor >> 8) & 0xFF) * oo255;
    constants.m_fogColor[2] = (fogColor & 0xFF) * oo255;
    constants.m_fogColor[3] = ((fogColor >> 24) & 0xFF) * oo255;

    if (!memcmp(&constants, &this->m_deviceConstants, sizeof(constants))) {
        return;
    }

    this->m_deviceConstants = constants;

    D3D11_MAPPED_SUBRESOURCE mapped;

    if (SUCCEEDED(this->m_d3dContext->Map(this->m_deviceConstantsBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
        memcpy(mapped.pData, &this->m_deviceConstants, sizeof(this->m_deviceConstants));
        this->m_d3dContext->Unmap(this->m_deviceConstantsBuffer, 0);
    }
}

void CGxDeviceD3d11::IStateSyncBlend() {
    if (!(this->m_stateDirty & StateDirty_Blend)) {
        return;
    }

    this->m_stateDirty &= ~StateDirty_Blend;

    uint32_t mode = this->m_appRenderStates[GxRs_BlendingMode].m_value.m_data.u[0];

    if (mode >= GxBlends_Last) {
        mode = GxBlend_Opaque;
    }

    // The Gx mask bits 0x1, 0x4, 0x2, 0x8 select red, green, blue, alpha.
    uint32_t value = (this->m_appMasterEnables & (1 << GxMasterEnable_ColorWrite)) ? this->m_appRenderStates[GxRs_ColorWrite].m_value.m_data.u[0] : 0;
    uint8_t writeMask = (value & 0x1) ? D3D11_COLOR_WRITE_ENABLE_RED : 0;

    if (value & 0x4) {
        writeMask |= D3D11_COLOR_WRITE_ENABLE_GREEN;
    }

    if (value & 0x2) {
        writeMask |= D3D11_COLOR_WRITE_ENABLE_BLUE;
    }

    if (value & 0x8) {
        writeMask |= D3D11_COLOR_WRITE_ENABLE_ALPHA;
    }

    uint64_t key = mode | (static_cast<uint64_t>(writeMask) << 8);
    auto it = this->m_blendStates.find(key);
    ID3D11BlendState* state;

    if (it != this->m_blendStates.end()) {
        state = it->second;
    } else {
        D3D11_BLEND_DESC desc;
        memset(&desc, 0, sizeof(desc));
        desc.RenderTarget[0].BlendEnable = mode >= GxBlend_Alpha;
        desc.RenderTarget[0].SrcBlend = s_srcBlend[mode];
        desc.RenderTarget[0].DestBlend = s_dstBlend[mode];
        desc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
        desc.RenderTarget[0].SrcBlendAlpha = s_srcBlendAlpha[mode];
        desc.RenderTarget[0].DestBlendAlpha = s_dstBlendAlpha[mode];
        desc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
        desc.RenderTarget[0].RenderTargetWriteMask = writeMask;

        if (FAILED(this->m_d3dDevice->CreateBlendState(&desc, &state))) {
            return;
        }

        this->m_blendStates[key] = state;
    }

    float factor = this->m_appRenderStates[GxRs_BlendFactor].m_value.m_data.f[0];
    const float factors[4] = { factor, factor, factor, factor };
    this->m_d3dContext->OMSetBlendState(state, factors, 0xFFFFFFFF);
    this->m_curBlendState = state;
}

void CGxDeviceD3d11::IStateSyncDepth() {
    if (!(this->m_stateDirty & StateDirty_Depth)) {
        return;
    }

    this->m_stateDirty &= ~StateDirty_Depth;

    bool test = (this->m_appMasterEnables & (1 << GxMasterEnable_DepthTest)) && this->m_appRenderStates[GxRs_DepthTest].m_value.m_data.i[0];
    bool write = (this->m_appMasterEnables & (1 << GxMasterEnable_DepthWrite)) && this->m_appRenderStates[GxRs_DepthWrite].m_value.m_data.i[0];
    uint32_t func = this->m_appRenderStates[GxRs_DepthFunc].m_value.m_data.u[0];

    if (func >= GxCmps_Last) {
        func = GxCmp_LEqual;
    }

    uint64_t key = func | (test ? 4 : 0) | (write ? 8 : 0);
    auto it = this->m_depthStates.find(key);
    ID3D11DepthStencilState* state;

    if (it != this->m_depthStates.end()) {
        state = it->second;
    } else {
        D3D11_DEPTH_STENCIL_DESC desc;
        memset(&desc, 0, sizeof(desc));
        desc.DepthEnable = TRUE;
        desc.DepthWriteMask = write ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
        desc.DepthFunc = test ? s_cmpFunc[func] : D3D11_COMPARISON_ALWAYS;

        if (FAILED(this->m_d3dDevice->CreateDepthStencilState(&desc, &state))) {
            return;
        }

        this->m_depthStates[key] = state;
    }

    this->m_d3dContext->OMSetDepthStencilState(state, 0);
    this->m_curDepthState = state;
}

void CGxDeviceD3d11::IStateSyncRasterizer() {
    if (!(this->m_stateDirty & StateDirty_Rasterizer)) {
        return;
    }

    this->m_stateDirty &= ~StateDirty_Rasterizer;

    uint32_t cull = 0;

    if (this->m_appMasterEnables & (1 << GxMasterEnable_Culling)) {
        cull = this->m_appRenderStates[GxRs_Culling].m_value.m_data.u[0];

        if (cull > 2) {
            cull = 2;
        }
    }

    bool fill = (this->m_appMasterEnables & (1 << GxMasterEnable_PolygonFill)) != 0;
    bool scissor = this->m_appRenderStates[GxRs_ScissorTest].m_value.m_data.i[0] != 0;
    bool multisample = this->m_sampleCount > 1 && this->m_appRenderStates[GxRs_Multisample].m_value.m_data.i[0] != 0;
    float polygonOffset = this->m_appRenderStates[GxRs_PolygonOffset].m_value.m_data.f[0];

    uint32_t offsetBits;
    memcpy(&offsetBits, &polygonOffset, sizeof(offsetBits));

    uint64_t key = cull | (fill ? 4 : 0) | (scissor ? 8 : 0) | (multisample ? 16 : 0) | (static_cast<uint64_t>(offsetBits) << 32);
    auto it = this->m_rasterizerStates.find(key);
    ID3D11RasterizerState* state;

    if (it != this->m_rasterizerStates.end()) {
        state = it->second;
    } else {
        static const D3D11_CULL_MODE cullModes[3] = { D3D11_CULL_NONE, D3D11_CULL_BACK, D3D11_CULL_FRONT };

        D3D11_RASTERIZER_DESC desc;
        memset(&desc, 0, sizeof(desc));
        desc.FillMode = fill ? D3D11_FILL_SOLID : D3D11_FILL_WIREFRAME;
        desc.CullMode = cullModes[cull];
        desc.FrontCounterClockwise = TRUE;
        desc.DepthBias = static_cast<INT>(polygonOffset * -16777216.0f);
        desc.DepthClipEnable = TRUE;
        desc.ScissorEnable = scissor;
        desc.MultisampleEnable = multisample;

        if (FAILED(this->m_d3dDevice->CreateRasterizerState(&desc, &state))) {
            return;
        }

        this->m_rasterizerStates[key] = state;
    }

    this->m_d3dContext->RSSetState(state);
    this->m_curRasterizerState = state;
}

void CGxDeviceD3d11::IStateSyncScissorRect() {
    if (!this->m_scissorRectDirty) {
        return;
    }

    const CRect& window = this->DeviceCurWindow();
    D3D11_RECT rect;

    rect.left = static_cast<LONG>(this->m_scissorRect.minX * window.maxX + 0.5f);
    rect.right = static_cast<LONG>(this->m_scissorRect.maxX * window.maxX + 1.0f);

    if (this->m_textureTarget[0].m_apiSpecific || this->m_textureTarget[1].m_apiSpecific) {
        rect.top = static_cast<LONG>(this->m_scissorRect.minY * window.maxY + 0.5f);
        rect.bottom = static_cast<LONG>(1.0f + this->m_scissorRect.maxY * window.maxY);
    } else {
        rect.top = static_cast<LONG>((1.0f - this->m_scissorRect.maxY) * window.maxY + 0.5f);
        rect.bottom = static_cast<LONG>(1.0f + (1.0f - this->m_scissorRect.minY) * window.maxY);
    }

    if (rect.left < 0) {
        rect.left = 0;
    }

    if (rect.top < 0) {
        rect.top = 0;
    }

    if (rect.right > window.maxX) {
        rect.right = static_cast<LONG>(window.maxX);
    }

    if (rect.bottom > window.maxY) {
        rect.bottom = static_cast<LONG>(window.maxY);
    }

    this->m_d3dContext->RSSetScissorRects(1, &rect);
    this->m_scissorRectDirty = 0;
}

void CGxDeviceD3d11::IXformSetViewport() {
    const CRect& window = this->DeviceCurWindow();
    D3D11_VIEWPORT viewport;

    float x = static_cast<float>(static_cast<int32_t>(this->m_viewport.m_x.m_l * window.maxX + 0.5f));
    float y = static_cast<float>(static_cast<int32_t>((1.0f - this->m_viewport.m_y.m_h) * window.maxY + 0.5f));
    viewport.TopLeftX = x;
    viewport.TopLeftY = y;
    viewport.Width = static_cast<float>(static_cast<int32_t>(this->m_viewport.m_x.m_h * window.maxX - x + 0.5f));
    viewport.Height = static_cast<float>(static_cast<int32_t>((1.0f - this->m_viewport.m_y.m_l) * window.maxY - y + 0.5f));
    viewport.MinDepth = this->m_viewport.m_z.m_l;
    viewport.MaxDepth = this->m_viewport.m_z.m_h;

    if (this->m_textureTarget[0].m_apiSpecific || this->m_textureTarget[1].m_apiSpecific) {
        viewport.TopLeftY = static_cast<float>(static_cast<int32_t>(window.maxY * this->m_viewport.m_y.m_l));
    }

    if (viewport.Width > 0.0f && viewport.Height > 0.0f) {
        this->m_d3dContext->RSSetViewports(1, &viewport);
    }

    this->m_viewportDirty = 0;
}

void CGxDeviceD3d11::XformSetProjection(const C44Matrix& matrix) {
    CGxDevice::XformSetProjection(matrix);

    C44Matrix projNative = matrix;

    if (!(fabs(projNative.c3 - 1.0f) < 0.00000023841858f) && !(fabs(projNative.c3) < 0.00000023841858f)) {
        projNative *= 1.0f / projNative.c3;
    }

    if (projNative.d3 == 0.0f) {
        float minZ = -(projNative.d2 / (projNative.c2 + 1.0f));
        float maxZ = -(projNative.d2 / (projNative.c2 - 1.0f));
        projNative.c2 = maxZ / (maxZ - minZ);
        projNative.d2 = maxZ * minZ / (minZ - maxZ);
    } else {
        float invC2 = 1.0f / projNative.c2;
        float minZ = (-1.0f - projNative.d2) * invC2;
        float maxZ = (1.0f - projNative.d2) * invC2;
        projNative.c2 = 1.0f / (maxZ - minZ);
        projNative.d2 = minZ / (minZ - maxZ);
    }

    if (!(this->m_appMasterEnables & (1 << GxMasterEnable_NormalProjection)) && projNative.d3 != 1.0f) {
        C44Matrix scale;
        scale.a0 = 0.2f;
        scale.b1 = 0.2f;
        scale.c2 = 0.2f;
        projNative = projNative * scale;
    }

    this->m_xforms[GxXform_Projection].m_dirty = 1;
    this->m_projNative = projNative;
}

void CGxDeviceD3d11::XformSetView(const C44Matrix& matrix) {
    CGxDevice::XformSetView(matrix);
}

void CGxDeviceD3d11::IShaderConstantsFlush() {
    struct Stage {
        ShadowConstants* m_shadow;
        float* m_mirror;
        ID3D11Buffer* m_buffer;
    };

    Stage stages[2] = {
        { &CGxDevice::s_shadowConstants[1], this->m_vsMirror, this->m_vsConstants },
        { &CGxDevice::s_shadowConstants[0], this->m_psMirror, this->m_psConstants },
    };

    for (uint32_t i = 0; i < 2; i++) {
        ShadowConstants& shadow = *stages[i].m_shadow;

        if (shadow.dirtyMin > shadow.dirtyMax) {
            continue;
        }

        memcpy(&stages[i].m_mirror[shadow.dirtyMin * 4], &shadow.constants[shadow.dirtyMin * 4], (shadow.dirtyMax - shadow.dirtyMin + 1) * 4 * sizeof(float));
        shadow.dirtyMin = 255;
        shadow.dirtyMax = 0;

        D3D11_MAPPED_SUBRESOURCE mapped;

        if (SUCCEEDED(this->m_d3dContext->Map(stages[i].m_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
            memcpy(mapped.pData, stages[i].m_mirror, 256 * 4 * sizeof(float));
            this->m_d3dContext->Unmap(stages[i].m_buffer, 0);
        }
    }
}

void CGxDeviceD3d11::IStateSyncInputLayout() {
    if (!this->m_curVsCode) {
        return;
    }

    // one input slot per source buffer
    CGxBuf* streamBufs[GxVAs_Last];
    uint32_t streamStrides[GxVAs_Last];
    uint32_t streamCount = 0;
    memset(streamBufs, 0, sizeof(streamBufs));
    memset(streamStrides, 0, sizeof(streamStrides));

    InputLayoutKey key;
    memset(&key, 0, sizeof(key));
    key.m_code = this->m_curVsCode;

    D3D11_INPUT_ELEMENT_DESC elements[GxVAs_Last];

    for (uint32_t i = 0; i < GxVAs_Last; i++) {
        if (!(this->m_primVertexMask & (1 << i))) {
            continue;
        }

        CGxBuf* buf = this->m_primVertexFormatBuf[i];
        uint32_t stream;

        for (stream = 0; stream < streamCount; stream++) {
            if (streamBufs[stream] == buf) {
                break;
            }
        }

        if (stream == streamCount) {
            streamBufs[streamCount++] = buf;
        }

        const CGxVertexAttrib& attrib = this->m_primVertexFormatAttrib[i];
        streamStrides[stream] += s_gxAttribToSize[attrib.m_type];

        D3D11_INPUT_ELEMENT_DESC& element = elements[key.m_count];
        element.SemanticName = s_gxAttribToSemanticName[attrib.m_attrib];
        element.SemanticIndex = s_gxAttribToSemanticIndex[attrib.m_attrib];
        element.Format = s_gxAttribToDxgiFormat[attrib.m_type];
        element.InputSlot = stream;
        element.AlignedByteOffset = attrib.m_offset;
        element.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
        element.InstanceDataStepRate = 0;

        key.m_elements[key.m_count * 2] = attrib.m_attrib | (stream << 8) | (attrib.m_type << 16);
        key.m_elements[key.m_count * 2 + 1] = attrib.m_offset;
        key.m_count++;
    }

    // preset formats can have a stride bigger than the sum of their attributes
    if (this->m_primVertexFormat < GxVertexBufferFormats_Last && streamCount == 1) {
        streamStrides[0] = this->m_primVertexSize;
    }

    auto it = this->m_inputLayouts.find(key);
    ID3D11InputLayout* layout = nullptr;

    if (it != this->m_inputLayouts.end()) {
        layout = it->second;
    } else if (SUCCEEDED(this->m_d3dDevice->CreateInputLayout(elements, key.m_count, this->m_curVsCode, this->m_curVsCodeSize, &layout))) {
        this->m_inputLayouts[key] = layout;
    } else {
        CGxDevice::Log("CGxDeviceD3d11: CreateInputLayout failed for vertex mask 0x%x", this->m_primVertexMask);
        this->m_inputLayouts[key] = nullptr;
    }

    if (!layout) {
        this->m_curInputLayout = nullptr;
        this->m_d3dContext->IASetInputLayout(nullptr);
        return;
    }

    if (this->m_curInputLayout != layout) {
        this->m_d3dContext->IASetInputLayout(layout);
        this->m_curInputLayout = layout;
    }

    ID3D11Buffer* buffers[GxVAs_Last];
    UINT offsets[GxVAs_Last];
    UINT strides[GxVAs_Last];
    bool changed = streamCount != this->m_curVertexBufferCount;

    for (uint32_t stream = 0; stream < streamCount; stream++) {
        CGxBuf* buf = streamBufs[stream];

        if (!buf->m_pool->m_apiSpecific) {
            this->ICreatePoolApi(buf->m_pool);
        }

        buffers[stream] = static_cast<ID3D11Buffer*>(buf->m_pool->m_apiSpecific);
        offsets[stream] = buf->m_index;
        strides[stream] = streamStrides[stream];

        if (buffers[stream] != this->m_curVertexBuffers[stream] || offsets[stream] != this->m_curVertexOffsets[stream] || strides[stream] != this->m_curVertexStrides[stream]) {
            changed = true;
        }
    }

    if (!changed) {
        return;
    }

    uint32_t bindCount = streamCount > this->m_curVertexBufferCount ? streamCount : this->m_curVertexBufferCount;

    for (uint32_t stream = streamCount; stream < bindCount; stream++) {
        buffers[stream] = nullptr;
        offsets[stream] = 0;
        strides[stream] = 0;
    }

    if (bindCount) {
        this->m_d3dContext->IASetVertexBuffers(0, bindCount, buffers, strides, offsets);
    }

    for (uint32_t stream = 0; stream < bindCount; stream++) {
        this->m_curVertexBuffers[stream] = buffers[stream];
        this->m_curVertexOffsets[stream] = offsets[stream];
        this->m_curVertexStrides[stream] = strides[stream];
    }

    this->m_curVertexBufferCount = streamCount;
}

void CGxDeviceD3d11::IStateSync() {
    this->IShaderConstantsFlush();
    this->IRsSync(0);

    if (this->m_appMasterEnables != this->m_hwMasterEnables) {
        int32_t enable;

        if (this->NeedsUpdate(this->m_appMasterEnables, this->m_hwMasterEnables, 0, 0, GxMasterEnable_PolygonFill, enable)) {
            this->m_stateDirty |= StateDirty_Rasterizer;
        }

        this->m_hwMasterEnables = this->m_appMasterEnables;
    }

    this->IStateSyncDeviceConstants();

    if (this->m_viewportDirty) {
        this->IXformSetViewport();
    }

    this->IStateSyncScissorRect();
    this->IStateSyncRasterizer();
    this->IStateSyncBlend();
    this->IStateSyncDepth();
    this->IBindShaders();

    for (uint32_t slot = 0; slot < 16; slot++) {
        this->ISetTexture(slot, static_cast<CGxTex*>(this->m_appRenderStates[GxRs_Texture0 + slot].m_value.m_data.p));
    }

    this->IStateSyncInputLayout();
}

int32_t CGxDeviceD3d11::IDrawSetup(EGxPrim primType) {
    if (!this->m_context || this->m_intF5C || !this->m_d3dContext) {
        return 0;
    }

    this->IStateSync();

    if (!this->m_curVertexShader || !this->m_curPixelShader || !this->m_curInputLayout) {
        return 0;
    }

    D3D11_PRIMITIVE_TOPOLOGY topology = s_primitiveTopology[primType];

    if (topology != this->m_curTopology) {
        this->m_d3dContext->IASetPrimitiveTopology(topology);
        this->m_curTopology = topology;
    }

    return 1;
}

void CGxDeviceD3d11::Draw(CGxBatch* batch, int32_t count) {
    if (!this->IDrawSetup(batch->m_primType)) {
        return;
    }

    if (!count) {
        if (batch->m_primType == GxPrim_TriangleFan) {
            this->IDrawFan(batch->m_count);
            return;
        }

        // the vertex buffer offset already points at the first vertex
        this->m_d3dContext->Draw(batch->m_count, 0);
        return;
    }

    if (batch->m_primType == GxPrim_TriangleFan) {
        return;
    }

    ID3D11Buffer* indexBuffer = static_cast<ID3D11Buffer*>(this->m_primIndexBuf->m_pool->m_apiSpecific);

    if (!indexBuffer) {
        return;
    }

    if (this->m_curIndexBuffer != indexBuffer) {
        this->m_d3dContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R16_UINT, 0);
        this->m_curIndexBuffer = indexBuffer;
    }

    this->m_d3dContext->DrawIndexed(batch->m_count, batch->m_start + (this->m_primIndexBuf->m_index >> 1), 0);
}

void CGxDeviceD3d11::IDrawFan(uint32_t vertexCount) {
    if (vertexCount < 3) {
        return;
    }

    uint32_t triangles = vertexCount - 2;
    uint32_t indexCount = triangles * 3;

    if (indexCount > 0xFFFF) {
        return;
    }

    if (!this->m_fanIndexBuffer) {
        D3D11_BUFFER_DESC desc;
        memset(&desc, 0, sizeof(desc));
        desc.ByteWidth = 0xFFFF * sizeof(uint16_t);
        desc.Usage = D3D11_USAGE_DYNAMIC;
        desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        if (FAILED(this->m_d3dDevice->CreateBuffer(&desc, nullptr, &this->m_fanIndexBuffer))) {
            return;
        }
    }

    D3D11_MAPPED_SUBRESOURCE mapped;

    if (FAILED(this->m_d3dContext->Map(this->m_fanIndexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
        return;
    }

    uint16_t* indices = static_cast<uint16_t*>(mapped.pData);

    for (uint32_t i = 0; i < triangles; i++) {
        indices[i * 3] = 0;
        indices[i * 3 + 1] = static_cast<uint16_t>(i + 1);
        indices[i * 3 + 2] = static_cast<uint16_t>(i + 2);
    }

    this->m_d3dContext->Unmap(this->m_fanIndexBuffer, 0);
    this->m_d3dContext->IASetIndexBuffer(this->m_fanIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
    this->m_curIndexBuffer = this->m_fanIndexBuffer;
    this->m_d3dContext->DrawIndexed(indexCount, 0, 0);
}
