file(GLOB_RECURSE PRIVATE_SOURCES CONFIGURE_DEPENDS "*.cpp")

find_program(GXMODERN_FXC fxc
    HINTS
        "$ENV{WindowsSdkVerBinPath}/x64"
        "C:/Program Files (x86)/Windows Kits/10/bin/${CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION}/x64"
)

if (NOT GXMODERN_FXC)
    message(FATAL_ERROR "fxc.exe not found (needed to compile the GxModern shaders). Install the Windows SDK.")
endif ()

set(GXMODERN_SHADER_OUT ${CMAKE_BINARY_DIR}/generated/GxModernShaders)
file(MAKE_DIRECTORY ${GXMODERN_SHADER_OUT})
set(GXMODERN_SHADER_OUTPUTS "")

# gxmodern_shader(<name> <file> <entry> <profile> [defines...])
function(gxmodern_shader name file entry profile)
    set(defines "")
    foreach (define ${ARGN})
        list(APPEND defines /D ${define})
    endforeach ()

    set(out ${GXMODERN_SHADER_OUT}/${name}.h)
    add_custom_command(
        OUTPUT ${out}
        COMMAND ${GXMODERN_FXC} /nologo /O3 /Zpr /T ${profile} /E ${entry} /Vn g_${name} /Fh ${out}
            /I ${CMAKE_SOURCE_DIR}/ThirdParty/smaa /I ${CMAKE_CURRENT_SOURCE_DIR}/Shaders ${defines}
            ${CMAKE_CURRENT_SOURCE_DIR}/Shaders/${file}
        DEPENDS
            ${CMAKE_CURRENT_SOURCE_DIR}/Shaders/${file}
            ${CMAKE_CURRENT_SOURCE_DIR}/Shaders/Common.hlsli
            ${CMAKE_SOURCE_DIR}/ThirdParty/smaa/SMAA.hlsl
        VERBATIM
    )
    set(GXMODERN_SHADER_OUTPUTS ${GXMODERN_SHADER_OUTPUTS} ${out} PARENT_SCOPE)
endfunction ()

gxmodern_shader(FullscreenVS Fullscreen.hlsl FullscreenVS vs_3_0)
gxmodern_shader(CopyPS Fullscreen.hlsl CopyPS ps_3_0)
gxmodern_shader(LinearDepthPS Gtao.hlsl LinearDepthPS ps_3_0)
gxmodern_shader(GtaoPS Gtao.hlsl GtaoPS ps_3_0)
gxmodern_shader(GtaoFullPS Gtao.hlsl GtaoPS ps_3_0 GTAO_FULL=1)
gxmodern_shader(AoBlurPS Gtao.hlsl AoBlurPS ps_3_0)
gxmodern_shader(AoApplyPS Gtao.hlsl AoApplyPS ps_3_0)
gxmodern_shader(FogMarchPS VolumeFog.hlsl FogMarchPS ps_3_0)
gxmodern_shader(FogMarchHighPS VolumeFog.hlsl FogMarchPS ps_3_0 FOG_STEPS=32)
gxmodern_shader(FogMarchShadowPS VolumeFog.hlsl FogMarchPS ps_3_0 GODRAYS=1 GODRAYS_HWPCF=1)
gxmodern_shader(FogMarchShadowHighPS VolumeFog.hlsl FogMarchPS ps_3_0 FOG_STEPS=32 GODRAYS=1 GODRAYS_HWPCF=1)
gxmodern_shader(FogMarchShadowRawPS VolumeFog.hlsl FogMarchPS ps_3_0 GODRAYS=1 GODRAYS_HWPCF=0)
gxmodern_shader(FogMarchShadowRawHighPS VolumeFog.hlsl FogMarchPS ps_3_0 FOG_STEPS=32 GODRAYS=1 GODRAYS_HWPCF=0)
gxmodern_shader(FogTemporalPS VolumeFog.hlsl FogTemporalPS ps_3_0)
gxmodern_shader(FogApplyPS VolumeFog.hlsl FogApplyPS ps_3_0)
gxmodern_shader(ColorGradePS ColorGrade.hlsl ColorGradePS ps_3_0)
gxmodern_shader(SmaaEdgeVS SmaaPasses.hlsl EdgeVS vs_3_0)
gxmodern_shader(SmaaEdgePS SmaaPasses.hlsl EdgePS ps_3_0)
gxmodern_shader(SmaaWeightsVS SmaaPasses.hlsl WeightsVS vs_3_0)
gxmodern_shader(SmaaWeightsPS SmaaPasses.hlsl WeightsPS ps_3_0)
gxmodern_shader(SmaaBlendVS SmaaPasses.hlsl BlendVS vs_3_0)
gxmodern_shader(SmaaBlendPS SmaaPasses.hlsl BlendPS ps_3_0)

add_custom_target(GxModernShaders DEPENDS ${GXMODERN_SHADER_OUTPUTS})

add_library(GxModern STATIC
    ${PRIVATE_SOURCES}
)

add_dependencies(GxModern GxModernShaders)

target_include_directories(GxModern
    PUBLIC
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/ThirdParty/smaa
        ${CMAKE_BINARY_DIR}/generated
)

target_link_libraries(GxModern
    PUBLIC
        imgui
        d3d9
)
