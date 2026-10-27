#include <windows.h>
#include <d3d9.h>
#include "Engine/Source/GxModern/GxModernInternal.h"
#include "GxModernShaders/FullscreenVS.h"
#include "GxModernShaders/CopyPS.h"
#include "GxModernShaders/LinearDepthPS.h"
#include "GxModernShaders/GtaoPS.h"
#include "GxModernShaders/GtaoFullPS.h"
#include "GxModernShaders/AoBlurPS.h"
#include "GxModernShaders/AoApplyPS.h"
#include "GxModernShaders/FogMarchPS.h"
#include "GxModernShaders/FogMarchHighPS.h"
#include "GxModernShaders/FogMarchShadowPS.h"
#include "GxModernShaders/FogMarchShadowHighPS.h"
#include "GxModernShaders/FogMarchShadowRawPS.h"
#include "GxModernShaders/FogMarchShadowRawHighPS.h"
#include "GxModernShaders/FogTemporalPS.h"
#include "GxModernShaders/FogApplyPS.h"
#include "GxModernShaders/ColorGradePS.h"
#include "GxModernShaders/SmaaEdgeVS.h"
#include "GxModernShaders/SmaaEdgePS.h"
#include "GxModernShaders/SmaaWeightsVS.h"
#include "GxModernShaders/SmaaWeightsPS.h"
#include "GxModernShaders/SmaaBlendVS.h"
#include "GxModernShaders/SmaaBlendPS.h"

namespace GxModern {

struct ShaderBlob {
    const BYTE* data;
    size_t size;
};

#define BLOB(name) { g_##name, sizeof(g_##name) }

static const ShaderBlob s_psBlobs[Ps_Count] = {
    BLOB(CopyPS),
    BLOB(LinearDepthPS),
    BLOB(GtaoPS),
    BLOB(GtaoFullPS),
    BLOB(AoBlurPS),
    BLOB(AoApplyPS),
    BLOB(FogMarchPS),
    BLOB(FogMarchHighPS),
    BLOB(FogMarchShadowPS),
    BLOB(FogMarchShadowHighPS),
    BLOB(FogMarchShadowRawPS),
    BLOB(FogMarchShadowRawHighPS),
    BLOB(FogTemporalPS),
    BLOB(FogApplyPS),
    BLOB(ColorGradePS),
    BLOB(SmaaEdgePS),
    BLOB(SmaaWeightsPS),
    BLOB(SmaaBlendPS),
};

static const ShaderBlob s_vsBlobs[Vs_Count] = {
    BLOB(FullscreenVS),
    BLOB(SmaaEdgeVS),
    BLOB(SmaaWeightsVS),
    BLOB(SmaaBlendVS),
};

#undef BLOB

static IDirect3DPixelShader9* s_pixelShaders[Ps_Count];
static IDirect3DVertexShader9* s_vertexShaders[Vs_Count];

IDirect3DPixelShader9* PixelShader(PsId id) {
    if (!s_pixelShaders[id]) {
        Device()->CreatePixelShader(reinterpret_cast<const DWORD*>(s_psBlobs[id].data), &s_pixelShaders[id]);
    }

    return s_pixelShaders[id];
}

IDirect3DVertexShader9* VertexShader(VsId id) {
    if (!s_vertexShaders[id]) {
        Device()->CreateVertexShader(reinterpret_cast<const DWORD*>(s_vsBlobs[id].data), &s_vertexShaders[id]);
    }

    return s_vertexShaders[id];
}

void ReleaseShaders() {
    for (IDirect3DPixelShader9*& shader : s_pixelShaders) {
        if (shader) {
            shader->Release();
            shader = nullptr;
        }
    }

    for (IDirect3DVertexShader9*& shader : s_vertexShaders) {
        if (shader) {
            shader->Release();
            shader = nullptr;
        }
    }
}

}
