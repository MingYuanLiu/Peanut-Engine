set(TARGET_NAME EngineShader)

set(SHADER_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(ALL_GENERATED_SPV_FILES "")
set(SHADER_TARGET_DIR "${PROJECT_BINARY_DIR}/spirv")
message(STATUS "Shader target: ${SHADER_TARGET_DIR}")

macro(compile_shader shader_name stage)
    set(SPV_FILE "${SHADER_TARGET_DIR}/${shader_name}.spv")
    set(SHADER_SRC_FILE "${SHADER_ROOT_DIR}/glsl/${shader_name}.${tag}")
    add_custom_command(
        OUTPUT ${SPV_FILE}
        COMMAND ${GLSLANGVALIDATOR_EXECUTABLE} -V -S ${stage} -o ${SPV_FILE} ${SHADER_SRC_FILE}
        DEPENDS ${SHADER_SRC_FILE}
    )
    list(APPEND ALL_GENERATED_SPV_FILES ${SPV_FILE})
endmacro()

file(MAKE_DIRECTORY ${SHADER_TARGET_DIR})
message(STATUS "Compile shaders start ")
compile_shader(equirect2cube comp)
compile_shader(irradiance_env_map comp)
compile_shader(pbr_fs frag)
compile_shader(pbr_vs vert)
compile_shader(skybox_fs frag)
compile_shader(skybox_vs vert)
compile_shader(spbrdf_cs comp)
compile_shader(spmap_cs comp)
compile_shader(tonemap_fs frag)
compile_shader(tonemap_vs vert)
message(STATUS "Compile shaders finished ")
add_custom_target(${TARGET_NAME} DEPENDS ${ALL_GENERATED_SPV_FILES})
install(DIRECTORY ${PROJECT_BINARY_DIR}/spirv DESTINATION assets/spirv)
