#ifndef ENGINE_SOURCE_GX_CGXDEVICED3D11_CGXDEVICED3D11_H
#define ENGINE_SOURCE_GX_CGXDEVICED3D11_CGXDEVICED3D11_H

#include <cstddef>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <dxgi.h>
#include "Engine/Source/Gx/CGxDevice/CGxDevice.h"
#include "Engine/Source/Gx/CGxTex.h"

class CGxDeviceD3d11 : public CGxDevice {
    public:
    // Owned by CGxTex::m_apiSpecificData.
    struct TexData {
        ID3D11Texture2D* m_texture;
        ID3D11ShaderResourceView* m_srv;
        ID3D11RenderTargetView* m_rtv[GxTexCubeFaces_Last];
        ID3D11DepthStencilView* m_dsv;
        uint32_t m_levels;
    };

    // Constant buffer shared by the vertex and pixel stage, register b1. Hand written shaders and generated ones read it.
    struct DeviceConstants {
        float m_alphaRef[4];
        float m_fog[4];
        float m_fogColor[4];
    };

    // Constant buffer of the generated shaders, register b2. Same layout as the cbuffer in CGxD3d11Emulation.cpp.
    struct EmuConstants {
        float m_worldView[16];
        float m_worldViewProj[16];
        float m_texMatrix[8][16];
        float m_matDiffuse[4];
        float m_matEmissive[4];
        float m_ambient[4];
        float m_lightPos[4][4];
        float m_lightDiffuse[4][4];
        float m_lightAmbient[4][4];
        float m_lightAtt[4][4];
        float m_world[16];
        float m_matSpecular[4];
        float m_lightSpec[4][4];
        float m_clipPlane[6][4];
    };

    struct EmuVertexShader {
        ID3D11VertexShader* m_shader;
        ID3DBlob* m_code;
    };

    struct EmuStageKey {
        uint8_t m_colorOp;
        uint8_t m_alphaOp;
        uint8_t m_texGen;
        uint8_t m_shaderMode;
        uint8_t m_coord;
        uint8_t m_transform;
    };

    struct EmuKey {
        uint32_t m_vertexMask;
        uint32_t m_lightMask;
        uint32_t m_pointMask;
        uint8_t m_lighting;
        uint8_t m_normalize;
        uint8_t m_materialFromColor;
        uint8_t m_emissiveFromColor;
        uint8_t m_stageCount;
        uint8_t m_clipMask;
        uint8_t m_specular;
        uint8_t m_pad;
        EmuStageKey m_stages[8];
    };

    struct InputLayoutKey {
        const void* m_code;
        uint32_t m_count;
        uint32_t m_elements[GxVAs_Last * 2];

        bool operator==(const InputLayoutKey& key) const;
    };

    struct InputLayoutKeyHash {
        size_t operator()(const InputLayoutKey& key) const;
    };

    enum {
        StateDirty_Blend = 0x1,
        StateDirty_Depth = 0x2,
        StateDirty_Rasterizer = 0x4,
        StateDirty_InputLayout = 0x8,
        StateDirty_DeviceConstants = 0x10,
    };

    /* window */
    HWND m_hwnd;
    ATOM m_hwndClass;
    int32_t m_ownhwnd;
    HCURSOR m_cursorHandle;
    bool m_inModeSwitch;

    /* device */
    ID3D11Device* m_d3dDevice;
    ID3D11DeviceContext* m_d3dContext;
    IDXGIFactory1* m_dxgiFactory;
    IDXGIAdapter1* m_dxgiAdapter;
    IDXGISwapChain* m_swapChain;
    DXGI_FORMAT m_backBufferFormat;
    uint32_t m_sampleCount;
    uint32_t m_backBufferWidth;
    uint32_t m_backBufferHeight;

    /* render targets */
    ID3D11RenderTargetView* m_backBufferRtv;
    ID3D11Texture2D* m_depthTexture;
    ID3D11DepthStencilView* m_depthDsv;
    ID3D11Texture2D* m_scratchDepthTexture;
    ID3D11DepthStencilView* m_scratchDepthDsv;
    uint32_t m_scratchDepthWidth;
    uint32_t m_scratchDepthHeight;
    ID3D11RenderTargetView* m_curRtv;
    ID3D11DepthStencilView* m_curDsv;
    ID3D11DepthStencilView* m_savedDsv;

    /* constants */
    ID3D11Buffer* m_vsConstants;
    ID3D11Buffer* m_psConstants;
    ID3D11Buffer* m_deviceConstantsBuffer;
    DeviceConstants m_deviceConstants;
    ID3D11Buffer* m_emuConstantsBuffer;
    EmuConstants m_emuConstants;
    ID3D11Buffer* m_clearConstants;
    CGxShader* m_clearShaders[2];
    ID3D11RasterizerState* m_clearRasterizer;
    ID3D11BlendState* m_clearBlend[2];
    ID3D11DepthStencilState* m_clearDepth[2];
    float m_vsMirror[256 * 4];
    float m_psMirror[256 * 4];

    /* pipeline state */
    uint32_t m_stateDirty;
    ID3D11InputLayout* m_curInputLayout;
    ID3D11VertexShader* m_curVertexShader;
    ID3D11PixelShader* m_curPixelShader;
    const void* m_curVsCode;
    size_t m_curVsCodeSize;
    ID3D11ShaderResourceView* m_boundSrv[16];
    ID3D11SamplerState* m_boundSampler[16];
    ID3D11Buffer* m_curIndexBuffer;
    ID3D11Buffer* m_curVertexBuffers[GxVAs_Last];
    uint32_t m_curVertexOffsets[GxVAs_Last];
    uint32_t m_curVertexStrides[GxVAs_Last];
    uint32_t m_curVertexBufferCount;
    ID3D11RasterizerState* m_curRasterizerState;
    ID3D11BlendState* m_curBlendState;
    ID3D11DepthStencilState* m_curDepthState;
    D3D11_PRIMITIVE_TOPOLOGY m_curTopology;
    ID3D11Buffer* m_fanIndexBuffer;
    uint32_t m_fanIndexCount;
    EmergencyMem m_staticEmergencyMem[2];
    ID3D11SamplerState* m_samplerStates[1024];
    ID3D11SamplerState* m_shadowSamplers[2];
    std::unordered_map<uint64_t, ID3D11RasterizerState*> m_rasterizerStates;
    std::unordered_map<uint64_t, ID3D11BlendState*> m_blendStates;
    std::unordered_map<uint64_t, ID3D11DepthStencilState*> m_depthStates;
    HMODULE m_compilerModule;
    pD3DCompile m_d3dCompile;
    std::unordered_map<std::string, EmuVertexShader> m_emuVertexShaders;
    std::unordered_map<std::string, ID3D11PixelShader*> m_emuPixelShaders;
    std::unordered_map<InputLayoutKey, ID3D11InputLayout*, InputLayoutKeyHash> m_inputLayouts;

    virtual void ITexMarkAsUpdated(CGxTex* texId);
    virtual void IRsSendToHw(EGxRenderState which);
    virtual void ICursorCreate(const CGxFormat& format);
    virtual void ICursorDestroy();
    virtual void ICursorDraw();
    virtual ~CGxDeviceD3d11();
    virtual int32_t DeviceCreate(GxWindowProc windowProc, const CGxFormat& format);
    virtual int32_t DeviceCreate(void* window, const CGxFormat& format);
    virtual void DeviceDestroy();
    virtual void DeviceEvictResources();
    virtual int32_t DeviceSetFormat(const CGxFormat& format);
    virtual void DeviceSetBaseMipLevel(uint32_t level);
    virtual void DeviceSetGamma(float gamma);
    virtual void DeviceSetGamma(const CGxGammaRamp& ramp);
    virtual void* DeviceWindow();
    virtual void DeviceReadPixels(CiRect& rect, TSGrowableArray<CImVector>& pixels);
    virtual void DeviceReadDepths(CiRect& rect, TSGrowableArray<float>& depths);
    virtual int32_t DeviceWM(EGxWM wm, uintptr_t param1, uintptr_t param2);
    virtual void DeviceSetRenderTarget(EGxBuffer buffer, CGxTex* texture, uint32_t plane);
    virtual void DeviceSaveDepthSurface();
    virtual void DeviceRestoreDepthSurface();
    virtual void DeviceResolveDepthBuffer(CGxTex* texture);
    virtual void DeviceCopyTex(CGxTex* source, uint32_t sourcePlane, CGxTex* dest, uint32_t destPlane);
    virtual void DeviceOverride(EGxOverride override, uint32_t value);
    virtual void CapsWindowSize(CRect& rect);
    virtual void CapsWindowSizeInScreenCoords(CRect& rect);
    virtual void LogCrashInfo(char* buffer, uint32_t size);
    virtual void ScenePresent();
    virtual void SceneClear(uint32_t mask, CImVector color);
    virtual void XformSetProjection(const C44Matrix& matrix);
    virtual void XformSetView(const C44Matrix& matrix);
    virtual void Draw(CGxBatch* batch, int32_t count);
    virtual void PoolSizeSet(CGxPool* pool, uint32_t size);
    virtual void PoolDestroy(CGxPool* pool);
    virtual char* BufLock(CGxBuf* buf);
    virtual int32_t BufUnlock(CGxBuf* buf, uint32_t size);
    virtual void BufData(CGxBuf* buf, const void* data, uint32_t size, uint32_t offset);
    virtual void TexDestroy(CGxTex* texId);
    virtual int32_t TexCopy(CGxTex* dest, const C2iVector& destPos, const C2iVector& sourcePos, const C2iVector& size, uint32_t level, uint32_t plane);
    virtual int32_t TexStretch(CGxTex* dest, const CiRect* destRect, const CiRect* sourceRect, uint32_t level, uint32_t face);
    virtual void QueryCreate(CGxQuery*& query, EGxQueryType type);
    virtual void QueryDestroy(CGxQuery*& query);
    virtual bool QueryBegin(CGxQuery* query);
    virtual bool QueryEnd(CGxQuery* query);
    virtual bool QueryGetParam(CGxQuery* query, EGxQueryParam param, uint32_t& data);
    virtual bool QueryGetData(CGxQuery* query, uint32_t& data);
    virtual void ShaderCreate(CGxShader** shaders, EGxShTarget target, const char* path, const char* name, uint32_t permutations);
    virtual void ShaderDestroy(CGxShader*& shader);
    virtual void IShaderReload(CGxShader* shader, const char* path, const char* name);
    virtual void IShaderCreate(CGxShader* shader);
    virtual void CursorSetVisible(int32_t visible);
    virtual void CursorUnlock(uint32_t x, uint32_t y);
    virtual void StereoSetConvergence(float convergence);
    virtual float StereoGetConvergence();
    virtual void StereoSetSeparation(float separation);
    virtual float StereoGetSeparation();
    virtual bool StereoEnabled();
    virtual void DeviceUnk81(uint32_t a, uint32_t b);
    virtual void DeviceUnk82();
    virtual void DeviceUnk83(uint32_t a, uint32_t b);

    static const DXGI_FORMAT s_GxFormatToDxgiFormat[CGxFormat::Formats_Last];
    static const DXGI_FORMAT s_GxFormatToDxgiTypeless[CGxFormat::Formats_Last];
    static const DXGI_FORMAT s_GxTexFormatToDxgiFormat[GxTexFormats_Last];
    static const DXGI_FORMAT s_GxTexFormatToSrvFormat[GxTexFormats_Last];
    static const DXGI_FORMAT s_gxAttribToDxgiFormat[7];
    static const uint32_t s_gxAttribToSize[7];
    static const char* const s_gxAttribToSemanticName[GxVAs_Last];
    static const uint32_t s_gxAttribToSemanticIndex[GxVAs_Last];
    static const D3D11_BLEND s_srcBlend[12];
    static const D3D11_BLEND s_srcBlendAlpha[12];
    static const D3D11_BLEND s_dstBlend[12];
    static const D3D11_BLEND s_dstBlendAlpha[12];
    static const D3D11_COMPARISON_FUNC s_cmpFunc[GxCmps_Last];
    static const D3D11_FILTER s_filterModes[GxTexFilters_Last];
    static const D3D11_PRIMITIVE_TOPOLOGY s_primitiveTopology[GxPrims_Last];

    static LRESULT CALLBACK WindowProcD3d11(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);

    CGxDeviceD3d11();
    int32_t ICreateWindow(CGxFormat& format);
    int32_t ICreateD3dDevice(const CGxFormat& format);
    int32_t ICreateSwapChain(const CGxFormat& format);
    int32_t ICreateTargets();
    void IReleaseTargets();
    void IDestroyD3d();
    void ISetCaps(const CGxFormat& format);
    void ISetWindowFocus(int32_t focus);
    void IResizeSwapChain(uint32_t width, uint32_t height);
    void IApplyGamma();
    void ISetRenderTargets();
    void ICursorBuild();
    int32_t ICreatePoolApi(CGxPool* pool);
    void IReleasePool(CGxPool* pool);
    void IReleaseD3dPools(int32_t freePools);
    void IReleaseD3dQueries();
    void QueryCreateApi(CGxQuery* query);
    void IStateReset();
    void IReleaseStateObjects();
    void IBindShaders();
    bool IEmuBind(ID3D11VertexShader*& vs, ID3D11PixelShader*& ps, const void*& vsCode, size_t& vsCodeSize);
    void IEmuBuildKey(EmuKey& key);
    void IEmuUpdateConstants(const EmuKey& key);
    void IEmuRelease();
    bool IEmuCompile(const std::string& source, const char* target, ID3DBlob** code);
    void IDrawFan(uint32_t vertexCount);
    void IUnbindTextures();
    void ICreateScratchDepth(uint32_t width, uint32_t height);
    int32_t ICopyBackBuffer(CGxTex* dest, const CiRect& sourceRect, const C2iVector& destPos, uint32_t level, uint32_t face);
    void IReleaseD3dShaders(int32_t freeShaders);
    char* IBufLock(CGxBuf* buf);
    void IBufUnlock(CGxBuf* buf, uint32_t size);
    TexData* ITexGetData(CGxTex* texId);
    int32_t ITexAllocD3dTexture(CGxTex* texId);
    void ITexAcquire(CGxTex* texId);
    void ITexCreate(CGxTex* texId);
    void ITexReleaseD3dTexture(CGxTex* texId);
    void ITexUploadRect(CGxTex* texId, uint32_t face, uint32_t level, const RECT& rect, const void* bits, uint32_t pitch);
    void ITexUploadAll(CGxTex* texId);
    void ITexUpload(CGxTex* texId);
    void ITexForceRecreation(int32_t all);
    int32_t IEnsureTexture(CGxTex* texId);
    void ISetTexture(uint32_t slot, CGxTex* texId);
    ID3D11SamplerState* IGetSamplerState(const CGxTexFlags& flags);
    ID3D11SamplerState* IGetShadowSamplerState(const CGxTexFlags& flags);
    void IShaderConstantsFlush();
    void IStateSync();
    void IStateSyncBlend();
    void IStateSyncDepth();
    void IStateSyncRasterizer();
    void IStateSyncScissorRect();
    void IStateSyncDeviceConstants();
    void IStateSyncInputLayout();
    void IXformSetViewport();
    bool ICalcViewport(D3D11_VIEWPORT& viewport);
    bool ISceneClearViewport(uint32_t mask, CImVector color);
    int32_t IDrawSetup(EGxPrim primType);
};

CGxDevice* CreateCGxDeviceD3d11();

#endif
