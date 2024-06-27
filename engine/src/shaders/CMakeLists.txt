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

file(GLOB_RECURSE ALL_FILES "*")

message(${CMAKE_CURRENT_SOURCE_DIR})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${ALL_FILES})

file(MAKE_DIRECTORY ${SHADER_TARGET_DIR})

# message(STATUS "Compile shaders start ")
# compile_shader(equirect2cube comp)
# compile_shader(irradiance_env_map comp)
# compile_shader(brdf_lut comp)
# compile_shader(prefiltered_env_map comp)
# compile_shader(color_grading frag)
# compile_shader(deferred_light frag)
# compile_shader(deferred_light vert)
# compile_shader(mesh_gbuffer frag)
# compile_shader(sceen vert)
# compile_shader(mesh vert)
# compile_shader(skybox vert)
# compile_shader(skybox frag)

# message(STATUS "Compile shaders finished ")
add_custom_target(${TARGET_NAME} SOURCES ${ALL_FILES})
# add_library(${TARGET_NAME} MODULE ${ALL_FILES})
# install(DIRECTORY ${PROJECT_BINARY_DIR}/spirv DESTINATION assets/spirv)
