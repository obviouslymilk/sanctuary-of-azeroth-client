#include <cstdint>
#include <cstring>
#include "Engine/Source/Gx/CGxDeviceD3d11/CGxDeviceD3d11.h"
#include "Engine/Source/Gx/CGxDevice/CGxShader.h"
#include "Engine/Source/Gx/Gx.h"
#include "Engine/Source/Storm/H/storm.h"

static void ReleaseShaderObject(CGxShader* shader) {
    if (shader->m_apiSpecific) {
        static_cast<IUnknown*>(shader->m_apiSpecific)->Release();
        shader->m_apiSpecific = nullptr;
    }

    shader->m_loaded = 0;
}

// OFFSET-15595: 0x00871220/0x1405A7050
void CGxDeviceD3d11::IShaderCreate(CGxShader* shader) {
    shader->m_valid = 0;

    if (!this->m_context) {
        return;
    }

    shader->m_loaded = 1;

    uint32_t size = shader->m_code.Count();

    if (!size) {
        return;
    }

    const void* code = shader->m_code.Ptr();
    HRESULT result = E_FAIL;

    if (shader->m_target == GxSh_Vertex) {
        result = this->m_d3dDevice->CreateVertexShader(code, size, nullptr, reinterpret_cast<ID3D11VertexShader**>(&shader->m_apiSpecific));
    } else if (shader->m_target == GxSh_Pixel) {
        result = this->m_d3dDevice->CreatePixelShader(code, size, nullptr, reinterpret_cast<ID3D11PixelShader**>(&shader->m_apiSpecific));
    }

    if (FAILED(result)) {
        shader->m_apiSpecific = nullptr;
        CGxDevice::Log("CGxDeviceD3d11::IShaderCreate(): %s shader '%s' rejected: 0x%08x", shader->m_target == GxSh_Vertex ? "vertex" : "pixel", shader->m_key.m_str, result);
        return;
    }

    shader->m_valid = 1;
}

void CGxDeviceD3d11::ShaderCreate(CGxShader** shaders, EGxShTarget target, const char* path, const char* name, uint32_t permutations) {
    CGxDevice::ShaderCreate(shaders, target, path, name, permutations);

    if (permutations == 1 && !shaders[0]->m_loaded) {
        this->IShaderCreate(shaders[0]);
    }
}

void CGxDeviceD3d11::ShaderDestroy(CGxShader*& shader) {
    if (shader->m_refCount == 1) {
        if (shader->m_apiSpecific == this->m_curVertexShader || shader->m_apiSpecific == this->m_curPixelShader) {
            this->m_curVertexShader = nullptr;
            this->m_curPixelShader = nullptr;
        }

        ReleaseShaderObject(shader);
    }

    CGxDevice::ShaderDestroy(shader);
}

void CGxDeviceD3d11::IShaderReload(CGxShader* shader, const char* path, const char* name) {
    if (shader->m_target != GxSh_Vertex && shader->m_target != GxSh_Pixel) {
        return;
    }

    if (shader->m_refCount == 1) {
        ReleaseShaderObject(shader);
    }

    this->IShaderLoad(&shader, static_cast<EGxShTarget>(shader->m_target), path, name, 1);
    this->IShaderCreate(shader);
}

void CGxDeviceD3d11::IReleaseD3dShaders(int32_t freeShaders) {
    if (!freeShaders) {
        return;
    }

    this->IEmuRelease();

    static const EGxShTarget targets[2] = { GxSh_Pixel, GxSh_Vertex };

    for (uint32_t i = 0; i < 2; i++) {
        TSHashTable<CGxShader, HASHKEY_STRI>& shaders = this->m_shaderList[targets[i]];

        for (CGxShader* shader = shaders.m_fulllist.Head(); shader; shader = shaders.m_fulllist.Next(shader)) {
            ReleaseShaderObject(shader);
        }
    }

    this->m_curVertexShader = nullptr;
    this->m_curPixelShader = nullptr;
}

void CGxDeviceD3d11::IBindShaders() {
    CGxShader* vertexShader = static_cast<CGxShader*>(this->m_appRenderStates[GxRs_VertexShader].m_value.m_data.p);
    CGxShader* pixelShader = static_cast<CGxShader*>(this->m_appRenderStates[GxRs_PixelShader].m_value.m_data.p);

    ID3D11VertexShader* vs = nullptr;
    ID3D11PixelShader* ps = nullptr;
    const void* vsCode = nullptr;
    size_t vsCodeSize = 0;

    if (!vertexShader && !pixelShader) {
        this->IEmuBind(vs, ps, vsCode, vsCodeSize);
    }

    if (vertexShader) {
        if (!vertexShader->m_loaded) {
            this->IShaderCreate(vertexShader);
        }

        if (vertexShader->m_valid) {
            vs = static_cast<ID3D11VertexShader*>(vertexShader->m_apiSpecific);
            vsCode = vertexShader->m_code.Ptr();
            vsCodeSize = vertexShader->m_code.Count();
        }
    }

    if (pixelShader) {
        if (!pixelShader->m_loaded) {
            this->IShaderCreate(pixelShader);
        }

        if (pixelShader->m_valid) {
            ps = static_cast<ID3D11PixelShader*>(pixelShader->m_apiSpecific);
        }
    }

    if (vs != this->m_curVertexShader) {
        this->m_d3dContext->VSSetShader(vs, nullptr, 0);
        this->m_curVertexShader = vs;
    }

    if (ps != this->m_curPixelShader) {
        this->m_d3dContext->PSSetShader(ps, nullptr, 0);
        this->m_curPixelShader = ps;
    }

    this->m_curVsCode = vsCode;
    this->m_curVsCodeSize = vsCodeSize;
}
