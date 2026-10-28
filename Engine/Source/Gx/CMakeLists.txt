file(GLOB_RECURSE PRIVATE_SOURCES CONFIGURE_DEPENDS "*.cpp")

if (NOT WIN32)
    list(FILTER PRIVATE_SOURCES EXCLUDE REGEX "/CGxDevice(D3d|D3d9Ex|OpenGL)/")
endif ()

# The D3D11 device is x64 only (decision of the DX11 plan); on Win32 GxApiSupported(GxApi_D3d11) reports 0.
if (NOT WIN32 OR NOT WOWDECOMP_ARCH_64)
    list(FILTER PRIVATE_SOURCES EXCLUDE REGEX "/CGxDeviceD3d11/")
endif ()

add_library(Gx STATIC
    ${PRIVATE_SOURCES}
)

target_include_directories(Gx
    PUBLIC
        ${CMAKE_SOURCE_DIR}
)

target_link_libraries(Gx
    PRIVATE
        d3d9.lib
)

if (WIN32)
    target_link_libraries(Gx
        PRIVATE
            nvapi
            opengl32.lib
    )
endif ()

if (WIN32 AND WOWDECOMP_ARCH_64)
    target_compile_definitions(Gx PUBLIC GX_HAS_D3D11=1)
    target_link_libraries(Gx
        PRIVATE
            d3d11.lib
            dxgi.lib
    )
endif ()

if (WIN32 AND WOWDECOMP_ARCH_64)
    file(GLOB FXC_CANDIDATES "C:/Program Files (x86)/Windows Kits/10/bin/*/x64/fxc.exe")
    list(SORT FXC_CANDIDATES ORDER DESCENDING)
    list(GET FXC_CANDIDATES 0 GX_FXC)
    find_package(Python3 REQUIRED COMPONENTS Interpreter)

    set(GX_SHADER_DIR ${CMAKE_CURRENT_SOURCE_DIR}/CGxDeviceD3d11/Shaders)
    set(GX_SHADER_OUT ${CMAKE_CURRENT_BINARY_DIR}/Shaders)
    set(GX_SHADER_PACK_ARGS)
    set(GX_SHADER_OUTPUTS)

    # gx_d3d11_shader(<Vertex|Pixel> <name> <permutations>): every permutation is built with -DPERMUTATION=<n>
    function(gx_d3d11_shader target name permutations)
        if (target STREQUAL "Vertex")
            set(profile vs_5_0)
        else ()
            set(profile ps_5_0)
        endif ()

        set(files)
        math(EXPR last "${permutations} - 1")
        foreach (i RANGE ${last})
            set(cso ${GX_SHADER_OUT}/${target}/${name}.${i}.cso)
            add_custom_command(
                OUTPUT ${cso}
                COMMAND ${CMAKE_COMMAND} -E make_directory ${GX_SHADER_OUT}/${target}
                COMMAND ${GX_FXC} /nologo /T ${profile} /E main /O3 /DPERMUTATION=${i} /I ${GX_SHADER_DIR} /Fo ${cso} ${GX_SHADER_DIR}/${target}/${name}.hlsl
                DEPENDS ${GX_SHADER_DIR}/${target}/${name}.hlsl ${GX_SHADER_DIR}/Common.hlsli
                COMMENT "fxc ${target}/${name} #${i}"
                VERBATIM
            )
            list(APPEND files ${cso})
        endforeach ()

        string(REPLACE ";" "," joined "${files}")
        set(GX_SHADER_PACK_ARGS ${GX_SHADER_PACK_ARGS} "${target}:${name}=${joined}" PARENT_SCOPE)
        set(GX_SHADER_OUTPUTS ${GX_SHADER_OUTPUTS} ${files} PARENT_SCOPE)
    endfunction ()

    gx_d3d11_shader(Vertex UI 2)
    gx_d3d11_shader(Pixel UI 1)
    gx_d3d11_shader(Pixel Desaturate 1)

    # world shaders, generated from the DX9 originals by Tools/Shaders/bls_to_hlsl.py
    gx_d3d11_shader(Vertex Terrain 128)
    gx_d3d11_shader(Pixel Terrain0 3)
    gx_d3d11_shader(Pixel Terrain0_env 1)
    gx_d3d11_shader(Pixel Terrain1 32)
    gx_d3d11_shader(Vertex DetailDoodad 6)
    gx_d3d11_shader(Pixel DetailDoodad 8)
    gx_d3d11_shader(Vertex vsLiquidWater 4)
    gx_d3d11_shader(Vertex vsLiquidWaterNoSpec 4)
    gx_d3d11_shader(Vertex vsLiquidMagma 1)
    gx_d3d11_shader(Pixel psLiquidWater 1)
    gx_d3d11_shader(Pixel psLiquidWaterNoSpec 1)
    gx_d3d11_shader(Pixel psLiquidMagma 1)
    gx_d3d11_shader(Vertex WaterRipples 1)
    gx_d3d11_shader(Pixel WaterRipples 1)

    set(GX_SHADER_TABLE ${CMAKE_CURRENT_BINARY_DIR}/GxEmbeddedShaderTable.cpp)
    add_custom_command(
        OUTPUT ${GX_SHADER_TABLE}
        COMMAND ${Python3_EXECUTABLE} ${GX_SHADER_DIR}/pack_shaders.py ${GX_SHADER_TABLE} ${GX_SHADER_PACK_ARGS}
        DEPENDS ${GX_SHADER_OUTPUTS} ${GX_SHADER_DIR}/pack_shaders.py
        COMMENT "Packing embedded D3D11 shaders"
        VERBATIM
    )
    target_sources(Gx PRIVATE ${GX_SHADER_TABLE})
endif ()
