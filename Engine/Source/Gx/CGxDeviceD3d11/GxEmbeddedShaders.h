#ifndef ENGINE_SOURCE_GX_CGXDEVICED3D11_GXEMBEDDEDSHADERS_H
#define ENGINE_SOURCE_GX_CGXDEVICED3D11_GXEMBEDDEDSHADERS_H

struct GxEmbeddedShader {
    const char* m_path;
    const unsigned char* m_data;
    unsigned int m_size;
};

extern const GxEmbeddedShader g_gxEmbeddedShaders[];
extern const unsigned int g_gxEmbeddedShaderCount;

#endif
