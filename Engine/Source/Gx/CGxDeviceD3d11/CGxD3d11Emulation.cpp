#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <string>
#include "Engine/Source/Gx/CGxDeviceD3d11/CGxDeviceD3d11.h"
#include "Engine/Source/Gx/CGxMatrixStack.h"
#include "Engine/Source/Gx/Gx.h"
#include "Engine/Source/ShaderEffect/Types.h"
#include "Engine/Source/Tempest/C44Matrix.h"

namespace {

enum {
    TexGen_ObjectLinear = 1,
    TexGen_World = 2,
    TexGen_Reflection = 4,
    TexGen_Normal = 5,
    TexGen_Sphere = 6,
};

enum {
    Lighting_Off = 0,
    Lighting_On = 1,
    Lighting_AmbientOnly = 2,
};

void Append(std::string& out, const char* format, ...) {
    char buffer[512];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    out += buffer;
}

bool UsesNormal(const CGxDeviceD3d11::EmuKey& key) {
    if (key.m_lighting != Lighting_Off && key.m_lightMask) {
        return true;
    }

    for (uint32_t i = 0; i < key.m_stageCount; i++) {
        if (key.m_stages[i].m_texGen == TexGen_Reflection || key.m_stages[i].m_texGen == TexGen_Normal || key.m_stages[i].m_texGen == TexGen_Sphere) {
            return true;
        }
    }

    return false;
}

const char* const s_prefix =
    "cbuffer Device : register(b1) { float4 alphaRef; float4 fog; float4 fogColor; };\n";

// Same layout as CGxDeviceD3d11::EmuConstants.
const char* const s_emuBuffer =
    "cbuffer Emu : register(b2) {\n"
    "    row_major float4x4 worldView;\n"
    "    row_major float4x4 worldViewProj;\n"
    "    row_major float4x4 texMatrix[8];\n"
    "    float4 matDiffuse;\n"
    "    float4 matEmissive;\n"
    "    float4 ambient;\n"
    "    float4 lightPos[4];\n"
    "    float4 lightDiffuse[4];\n"
    "    float4 lightAmbient[4];\n"
    "    float4 lightAtt[4];\n"
    "    row_major float4x4 world;\n"
    "    float4 matSpecular;\n"
    "    float4 lightSpec[4];\n"
    "    float4 clipPlane[6];\n"
    "};\n";

std::string BuildVertexSource(const CGxDeviceD3d11::EmuKey& key) {
    std::string src = s_prefix;
    src += s_emuBuffer;

    bool hasColor = (key.m_vertexMask & (1 << GxVA_Color0)) != 0;
    bool hasNormal = (key.m_vertexMask & (1 << GxVA_Normal)) != 0;

    src += "struct VsIn {\n    float4 pos : POSITION;\n";

    if (hasNormal) {
        src += "    float4 normal : NORMAL;\n";
    }

    if (hasColor) {
        src += "    float4 color : COLOR0;\n";
    }

    for (uint32_t i = 0; i < 8; i++) {
        if (key.m_vertexMask & (1 << (GxVA_TexCoord0 + i))) {
            Append(src, "    float4 uv%u : TEXCOORD%u;\n", i, i);
        }
    }

    src += "};\nstruct VsOut {\n    float4 position : SV_Position;\n    float4 color : COLOR0;\n";

    if (key.m_specular) {
        src += "    float3 specular : COLOR1;\n";
    }

    if (key.m_clipMask) {
        src += "    float4 clip0 : SV_ClipDistance0;\n    float2 clip1 : SV_ClipDistance1;\n";
    }

    for (uint32_t i = 0; i < key.m_stageCount; i++) {
        Append(src, "    float4 t%u : TEXCOORD%u;\n", i, i);
    }

    src += "    float fogFactor : TEXCOORD8;\n};\n";
    src += "VsOut main(VsIn i) {\n    VsOut o;\n";
    src += "    float4 p = float4(i.pos.xyz, 1.0f);\n";
    src += "    o.position = mul(p, worldViewProj);\n";
    src += "    float3 viewPos = mul(p, worldView).xyz;\n";

    if (key.m_clipMask) {
        src += "    float4 wp = mul(p, world);\n";
        src += "    o.clip0 = float4(dot(wp, clipPlane[0]), dot(wp, clipPlane[1]), dot(wp, clipPlane[2]), dot(wp, clipPlane[3]));\n";
        src += "    o.clip1 = float2(dot(wp, clipPlane[4]), dot(wp, clipPlane[5]));\n";
    }
    src += "    o.fogFactor = saturate((fog.y - abs(viewPos.z)) * fog.w);\n";

    if (UsesNormal(key)) {
        if (hasNormal) {
            src += "    float3 n = mul(float4(i.normal.xyz, 0.0f), worldView).xyz;\n";
        } else {
            src += "    float3 n = float3(0.0f, 0.0f, 1.0f);\n";
        }

        if (key.m_normalize) {
            src += "    n = normalize(n);\n";
        }
    }

    src += hasColor ? "    float4 vcolor = i.color;\n" : "    float4 vcolor = float4(1.0f, 1.0f, 1.0f, 1.0f);\n";

    if (key.m_lighting == Lighting_Off) {
        src += "    o.color = vcolor;\n";
    } else {
        if (key.m_specular) {
            src += "    float3 spec = float3(0.0f, 0.0f, 0.0f);\n    float3 eye = -normalize(viewPos);\n";
        }

        src += key.m_materialFromColor ? "    float4 md = vcolor;\n" : "    float4 md = matDiffuse;\n";
        src += key.m_emissiveFromColor ? "    float3 lit = vcolor.rgb + md.rgb * ambient.rgb;\n" : "    float3 lit = matEmissive.rgb + md.rgb * ambient.rgb;\n";

        for (uint32_t i = 0; i < 4; i++) {
            if (!(key.m_lightMask & (1 << i))) {
                continue;
            }

            src += "    {\n        float3 l;\n        float att = 1.0f;\n";

            if (key.m_pointMask & (1 << i)) {
                Append(src, "        float3 d = lightPos[%u].xyz - viewPos;\n", i);
                src += "        float dist = length(d);\n        l = d / max(dist, 0.000001f);\n";
                Append(src, "        att = 1.0f / (lightDiffuse[%u].a + lightAmbient[%u].a * dist + lightAtt[%u].x * dist * dist);\n", i, i, i);
            } else {
                Append(src, "        l = normalize(-lightPos[%u].xyz);\n", i);
            }

            Append(src, "        lit += att * md.rgb * (lightAmbient[%u].rgb + lightDiffuse[%u].rgb * max(dot(n, l), 0.0f));\n", i, i);

            if (key.m_specular) {
                Append(src, "        float3 h = normalize(l + eye);\n        spec += att * lightSpec[%u].rgb * matSpecular.rgb * pow(max(dot(n, h), 0.0f), matSpecular.w) * (dot(n, l) > 0.0f ? 1.0f : 0.0f);\n", i);
            }
            src += "    }\n";
        }

        src += "    o.color = float4(saturate(lit), md.a);\n";

        if (key.m_specular) {
            src += "    o.specular = saturate(spec);\n";
        }
    }

    for (uint32_t i = 0; i < key.m_stageCount; i++) {
        const CGxDeviceD3d11::EmuStageKey& stage = key.m_stages[i];

        src += "    {\n";

        switch (stage.m_texGen) {
            case TexGen_ObjectLinear:
            case TexGen_World:
            case 3:
                src += "        float4 s = float4(viewPos, 1.0f);\n";
                break;

            case TexGen_Reflection:
            case TexGen_Sphere:
                src += "        float4 s = float4(reflect(normalize(viewPos), n), 1.0f);\n";
                break;

            case TexGen_Normal:
                src += "        float4 s = float4(n, 1.0f);\n";
                break;

            default:
                if (stage.m_coord < 8 && (key.m_vertexMask & (1 << (GxVA_TexCoord0 + stage.m_coord)))) {
                    Append(src, "        float4 s = i.uv%u;\n", stage.m_coord);
                } else {
                    src += "        float4 s = float4(0.0f, 0.0f, 0.0f, 1.0f);\n";
                }

                src += stage.m_shaderMode == 1 ? "        s = float4(s.xy, 0.0f, 1.0f);\n" : "        s = float4(s.xy, 1.0f, 0.0f);\n";
                break;
        }

        if (stage.m_transform) {
            Append(src, "        o.t%u = mul(s, texMatrix[%u]);\n", i, i);
        } else {
            Append(src, "        o.t%u = s;\n", i);
        }

        src += "    }\n";
    }

    src += "    return o;\n}\n";
    return src;
}

std::string BuildPixelSource(const CGxDeviceD3d11::EmuKey& key) {
    std::string src = s_prefix;

    for (uint32_t i = 0; i < key.m_stageCount; i++) {
        Append(src, "Texture2D tex%u : register(t%u);\nSamplerState smp%u : register(s%u);\n", i, i, i, i);
    }

    src += "struct PsIn {\n    float4 position : SV_Position;\n    float4 color : COLOR0;\n";

    if (key.m_specular) {
        src += "    float3 specular : COLOR1;\n";
    }

    for (uint32_t i = 0; i < key.m_stageCount; i++) {
        Append(src, "    float4 t%u : TEXCOORD%u;\n", i, i);
    }

    src += "    float fogFactor : TEXCOORD8;\n};\n";
    src += "float4 main(PsIn i) : SV_Target {\n    float4 cur = i.color;\n    float d = i.color.a;\n";

    // rows are indexed by EGxTexOp: Mod, Mod2x, Add, PassThru, Decal, Fade
    static const char* const colorExpr[GxTexOps_Last] = {
        "tx.rgb * prev.rgb",
        "saturate(tx.rgb * prev.rgb * 2.0f)",
        "saturate(tx.rgb + prev.rgb)",
        "prev.rgb",
        "prev.rgb * prev.a + tx.rgb * (1.0f - prev.a)",
        "tx.rgb * d + prev.rgb * (1.0f - d)",
    };

    static const char* const alphaExpr[GxTexOps_Last] = {
        "tx.a * prev.a",
        "saturate(tx.a * prev.a * 2.0f)",
        "saturate(tx.a + prev.a)",
        "prev.a",
        "prev.a * prev.a + tx.a * (1.0f - prev.a)",
        "tx.a * d + prev.a * (1.0f - d)",
    };

    for (uint32_t i = 0; i < key.m_stageCount; i++) {
        const CGxDeviceD3d11::EmuStageKey& stage = key.m_stages[i];
        uint32_t colorOp = stage.m_colorOp < GxTexOps_Last ? stage.m_colorOp : 0;
        uint32_t alphaOp = stage.m_alphaOp < GxTexOps_Last ? stage.m_alphaOp : 0;

        src += "    {\n";

        if (stage.m_shaderMode == 2) {
            Append(src, "        float2 uv = i.t%u.xy / i.t%u.z;\n", i, i);
        } else {
            Append(src, "        float2 uv = i.t%u.xy;\n", i);
        }

        Append(src, "        float4 tx = tex%u.Sample(smp%u, uv);\n        float4 prev = cur;\n", i, i);
        Append(src, "        cur.rgb = %s;\n        cur.a = %s;\n", colorExpr[colorOp], alphaExpr[alphaOp]);
        src += "        cur = saturate(cur);\n    }\n";
    }

    if (key.m_specular) {
        src += "    cur.rgb = saturate(cur.rgb + i.specular);\n";
    }

    src += "    if (fog.z != 0.0f) {\n        cur.rgb = lerp(fogColor.rgb, cur.rgb, i.fogFactor);\n    }\n";
    src += "    if (alphaRef.y != 0.0f) {\n        clip(cur.a * 255.0f - alphaRef.x);\n    }\n";
    src += "    return cur;\n}\n";
    return src;
}

void CopyMatrix(float* dest, const C44Matrix& matrix) {
    memcpy(dest, &matrix, sizeof(float) * 16);
}

void UnpackColor(float* dest, uint32_t color) {
    dest[0] = ((color >> 16) & 0xFF) * (1.0f / 255.0f);
    dest[1] = ((color >> 8) & 0xFF) * (1.0f / 255.0f);
    dest[2] = (color & 0xFF) * (1.0f / 255.0f);
    dest[3] = ((color >> 24) & 0xFF) * (1.0f / 255.0f);
}

} // namespace

void CGxDeviceD3d11::IEmuBuildKey(EmuKey& key) {
    memset(&key, 0, sizeof(key));

    uint32_t mask = this->m_primVertexMask;
    bool hasColor = (mask & (1 << GxVA_Color0)) != 0;
    uint32_t colorMaterial = this->m_appRenderStates[GxRs_ColorMaterial].m_value.m_data.u[0];

    if (this->m_appRenderStates[GxRs_Lighting].m_value.m_data.i[0]) {
        key.m_lighting = (this->m_appMasterEnables & (1 << GxMasterEnable_Lighting)) ? Lighting_On : Lighting_Off;
    } else if (hasColor || this->m_appRenderStates[GxRs_MatDiffuse].m_value.m_data.u[0] == 0xFFFFFFFF) {
        key.m_lighting = Lighting_Off;
    } else {
        key.m_lighting = Lighting_AmbientOnly;
    }

    if (key.m_lighting == Lighting_On) {
        for (uint32_t i = 0; i < 4; i++) {
            if (this->m_lights[i].m_enable) {
                key.m_lightMask |= 1 << i;

                if (this->m_lights[i].m_dir.w == 1.0f) {
                    key.m_pointMask |= 1 << i;
                }
            }
        }
    }

    if (key.m_lighting != Lighting_Off && hasColor) {
        key.m_materialFromColor = colorMaterial == 0;
        key.m_emissiveFromColor = colorMaterial == 2;
    }

    key.m_normalize = this->m_appRenderStates[GxRs_NormalizeNormals].m_value.m_data.u[0] != 0;
    key.m_clipMask = static_cast<uint8_t>(this->m_appRenderStates[GxRs_ClipPlaneMask].m_value.m_data.u[0] & 0x3F);

    // same condition as the D3D9 specular enable
    key.m_specular = key.m_lighting == Lighting_On && key.m_lightMask && this->m_appRenderStates[GxRs_MatSpecular].m_value.m_data.u[0] && this->m_appRenderStates[GxRs_MatSpecularExp].m_value.m_data.f[0] > 0.0f;

    uint32_t stageCount = 0;

    while (stageCount < 8 && this->m_appRenderStates[GxRs_Texture0 + stageCount].m_value.m_data.p) {
        stageCount++;
    }

    key.m_stageCount = static_cast<uint8_t>(stageCount);

    uint32_t usedMask = (1 << GxVA_Position) | (hasColor ? (1 << GxVA_Color0) : 0);

    for (uint32_t i = 0; i < stageCount; i++) {
        EmuStageKey& stage = key.m_stages[i];
        stage.m_colorOp = static_cast<uint8_t>(this->m_appRenderStates[GxRs_ColorOp0 + i].m_value.m_data.u[0]);
        stage.m_alphaOp = static_cast<uint8_t>(this->m_appRenderStates[GxRs_AlphaOp0 + i].m_value.m_data.u[0]);
        stage.m_texGen = static_cast<uint8_t>(this->m_appRenderStates[GxRs_TexGen0 + i].m_value.m_data.u[0]);
        stage.m_shaderMode = static_cast<uint8_t>(this->m_appRenderStates[GxRs_TextureShader0 + i].m_value.m_data.u[0]);
        stage.m_coord = static_cast<uint8_t>(this->m_appRenderStates[GxRs_TextureCoord0 + i].m_value.m_data.u[0]);
        stage.m_transform = stage.m_shaderMode != 0 || stage.m_texGen == TexGen_ObjectLinear || stage.m_texGen == TexGen_World || stage.m_texGen == TexGen_Sphere;

        if (!stage.m_texGen && stage.m_coord < 8) {
            usedMask |= 1 << (GxVA_TexCoord0 + stage.m_coord);
        }
    }

    if (UsesNormal(key)) {
        usedMask |= 1 << GxVA_Normal;
    }

    key.m_vertexMask = mask & usedMask;
}

void CGxDeviceD3d11::IEmuUpdateConstants(const EmuKey& key) {
    EmuConstants constants = this->m_emuConstants;

    CGxMatrixStack& worldStack = this->m_xforms[GxXform_World];
    CGxMatrixStack& viewStack = this->m_xforms[GxXform_View];
    const C44Matrix& world = worldStack.m_mtx[worldStack.m_level];
    const C44Matrix& view = viewStack.m_mtx[viewStack.m_level];
    C44Matrix worldView = world * view;

    CopyMatrix(constants.m_worldView, worldView);
    CopyMatrix(constants.m_world, world);
    CopyMatrix(constants.m_worldViewProj, worldView * this->m_projNative);

    for (uint32_t i = 0; i < key.m_stageCount; i++) {
        const EmuStageKey& stage = key.m_stages[i];

        if (!stage.m_transform) {
            continue;
        }

        C44Matrix matrix;

        if (stage.m_texGen == TexGen_ObjectLinear || stage.m_texGen == TexGen_World) {
            matrix = view.AffineInverse();

            if (stage.m_texGen == TexGen_ObjectLinear) {
                matrix = matrix * world.Inverse();
            }
        } else if (stage.m_texGen == TexGen_Sphere) {
            matrix.a0 = 0.5f;
            matrix.b1 = 0.5f;
            matrix.d0 = 0.5f;
            matrix.d1 = 0.5f;
        }

        if (stage.m_shaderMode != 0) {
            CGxMatrixStack& xform = this->m_xforms[i];
            matrix = matrix * xform.m_mtx[xform.m_level];
        }

        CopyMatrix(constants.m_texMatrix[i], matrix);
    }

    UnpackColor(constants.m_matDiffuse, this->m_appRenderStates[GxRs_MatDiffuse].m_value.m_data.u[0]);
    UnpackColor(constants.m_matEmissive, this->m_appRenderStates[GxRs_MatEmissive].m_value.m_data.u[0]);

    UnpackColor(constants.m_matSpecular, this->m_appRenderStates[GxRs_MatSpecular].m_value.m_data.u[0]);
    constants.m_matSpecular[3] = this->m_appRenderStates[GxRs_MatSpecularExp].m_value.m_data.f[0];

    for (uint32_t i = 0; i < 6; i++) {
        const C4Plane& plane = this->m_clipPlane[i];
        constants.m_clipPlane[i][0] = plane.n.x;
        constants.m_clipPlane[i][1] = plane.n.y;
        constants.m_clipPlane[i][2] = plane.n.z;
        constants.m_clipPlane[i][3] = plane.d;
    }

    float ambient = key.m_lighting == Lighting_AmbientOnly ? 1.0f : 0.0f;
    constants.m_ambient[0] = constants.m_ambient[1] = constants.m_ambient[2] = constants.m_ambient[3] = ambient;

    // lights are kept in view space as of the last time they changed
    if (key.m_lighting == Lighting_On) {
        for (uint32_t i = 0; i < 4; i++) {
            CGxApiLight& light = this->m_lights[i];

            if (!light.m_flags || !light.m_enable) {
                continue;
            }

            bool point = light.m_dir.w == 1.0f;
            float w = point ? 1.0f : 0.0f;
            const float* m = reinterpret_cast<const float*>(&view);

            for (uint32_t c = 0; c < 3; c++) {
                constants.m_lightPos[i][c] = light.m_dir.x * m[c] + light.m_dir.y * m[4 + c] + light.m_dir.z * m[8 + c] + w * m[12 + c];
            }

            constants.m_lightPos[i][3] = w;
            constants.m_lightDiffuse[i][0] = light.m_dirColor.x;
            constants.m_lightDiffuse[i][1] = light.m_dirColor.y;
            constants.m_lightDiffuse[i][2] = light.m_dirColor.z;
            constants.m_lightDiffuse[i][3] = light.m_constantAttenuation;
            constants.m_lightAmbient[i][0] = light.m_ambColor.x;
            constants.m_lightAmbient[i][1] = light.m_ambColor.y;
            constants.m_lightAmbient[i][2] = light.m_ambColor.z;
            constants.m_lightAmbient[i][3] = light.m_linearAttenuation;
            constants.m_lightAtt[i][0] = light.m_quadraticAttenuation;
            constants.m_lightSpec[i][0] = light.m_specColor.x;
            constants.m_lightSpec[i][1] = light.m_specColor.y;
            constants.m_lightSpec[i][2] = light.m_specColor.z;
            light.m_flags = 0;
        }
    }

    if (!memcmp(&constants, &this->m_emuConstants, sizeof(constants))) {
        return;
    }

    this->m_emuConstants = constants;

    D3D11_MAPPED_SUBRESOURCE mapped;

    if (SUCCEEDED(this->m_d3dContext->Map(this->m_emuConstantsBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
        memcpy(mapped.pData, &this->m_emuConstants, sizeof(this->m_emuConstants));
        this->m_d3dContext->Unmap(this->m_emuConstantsBuffer, 0);
    }
}

bool CGxDeviceD3d11::IEmuCompile(const std::string& source, const char* target, ID3DBlob** code) {
    if (!this->m_d3dCompile) {
        this->m_compilerModule = LoadLibraryExA("d3dcompiler_47.dll", nullptr, LOAD_LIBRARY_SEARCH_SYSTEM32);

        if (!this->m_compilerModule) {
            CGxDevice::Log("CGxDeviceD3d11: d3dcompiler_47.dll not found, no shaders for the fixed function pipeline");
            return false;
        }

        this->m_d3dCompile = reinterpret_cast<pD3DCompile>(GetProcAddress(this->m_compilerModule, "D3DCompile"));

        if (!this->m_d3dCompile) {
            return false;
        }
    }

    ID3DBlob* errors = nullptr;
    HRESULT result = this->m_d3dCompile(source.data(), source.size(), nullptr, nullptr, nullptr, "main", target, D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, code, &errors);

    if (FAILED(result)) {
        CGxDevice::Log("CGxDeviceD3d11: fixed function %s shader failed to compile: 0x%08x %s", target, result, errors ? static_cast<const char*>(errors->GetBufferPointer()) : "");
    }

    if (errors) {
        errors->Release();
    }

    return SUCCEEDED(result);
}

bool CGxDeviceD3d11::IEmuBind(ID3D11VertexShader*& vs, ID3D11PixelShader*& ps, const void*& vsCode, size_t& vsCodeSize) {
    EmuKey key;
    this->IEmuBuildKey(key);

    EmuKey vertexKey = key;
    EmuKey pixelKey;
    memset(&pixelKey, 0, sizeof(pixelKey));
    pixelKey.m_stageCount = key.m_stageCount;
    pixelKey.m_specular = key.m_specular;

    for (uint32_t i = 0; i < key.m_stageCount; i++) {
        vertexKey.m_stages[i].m_colorOp = 0;
        vertexKey.m_stages[i].m_alphaOp = 0;
        pixelKey.m_stages[i].m_colorOp = key.m_stages[i].m_colorOp;
        pixelKey.m_stages[i].m_alphaOp = key.m_stages[i].m_alphaOp;
        pixelKey.m_stages[i].m_shaderMode = key.m_stages[i].m_shaderMode == 2;
    }

    std::string vertexName(reinterpret_cast<const char*>(&vertexKey), sizeof(vertexKey));
    auto vertexIt = this->m_emuVertexShaders.find(vertexName);

    if (vertexIt == this->m_emuVertexShaders.end()) {
        EmuVertexShader entry = {};
        ID3DBlob* code = nullptr;

        if (this->IEmuCompile(BuildVertexSource(vertexKey), "vs_5_0", &code)) {
            if (SUCCEEDED(this->m_d3dDevice->CreateVertexShader(code->GetBufferPointer(), code->GetBufferSize(), nullptr, &entry.m_shader))) {
                entry.m_code = code;
            } else {
                code->Release();
            }
        }

        vertexIt = this->m_emuVertexShaders.emplace(vertexName, entry).first;
    }

    std::string pixelName(reinterpret_cast<const char*>(&pixelKey), sizeof(pixelKey));
    auto pixelIt = this->m_emuPixelShaders.find(pixelName);

    if (pixelIt == this->m_emuPixelShaders.end()) {
        ID3D11PixelShader* shader = nullptr;
        ID3DBlob* code = nullptr;

        if (this->IEmuCompile(BuildPixelSource(pixelKey), "ps_5_0", &code)) {
            this->m_d3dDevice->CreatePixelShader(code->GetBufferPointer(), code->GetBufferSize(), nullptr, &shader);
            code->Release();
        }

        pixelIt = this->m_emuPixelShaders.emplace(pixelName, shader).first;
    }

    vs = vertexIt->second.m_shader;
    ps = pixelIt->second;

    if (!vs || !ps) {
        return false;
    }

    vsCode = vertexIt->second.m_code->GetBufferPointer();
    vsCodeSize = vertexIt->second.m_code->GetBufferSize();
    this->IEmuUpdateConstants(key);
    return true;
}

void CGxDeviceD3d11::IEmuRelease() {
    for (auto& entry : this->m_emuVertexShaders) {
        if (entry.second.m_shader) {
            entry.second.m_shader->Release();
        }

        if (entry.second.m_code) {
            entry.second.m_code->Release();
        }
    }

    this->m_emuVertexShaders.clear();

    for (auto& entry : this->m_emuPixelShaders) {
        if (entry.second) {
            entry.second->Release();
        }
    }

    this->m_emuPixelShaders.clear();

    // layouts are keyed by the address of the vertex shader bytecode
    for (auto& entry : this->m_inputLayouts) {
        if (entry.second) {
            entry.second->Release();
        }
    }

    this->m_inputLayouts.clear();
    this->m_curInputLayout = nullptr;
    this->m_stateDirty |= StateDirty_InputLayout;
}
