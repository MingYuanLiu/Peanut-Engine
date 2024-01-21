set(TARGET_NAME PeanutRuntime)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED true)

# Add json library
set(JSON_INCLUDE ${THIRD_PARTY_DIR}/json11)
add_library(json11 ${JSON_INCLUDE}/json11.cpp)
set_target_properties(json11 PROPERTIES FOLDER "ThirdParty/json11")

# Add volk library
# set(VOLK_DIR ${THIRD_PARTY_DIR}/volk)
# set(VOLK_INCLUDE ${VOLK_DIR}/include)
# set(VOLK_SRC ${VOLK_DIR}/src/volk.c)

# Add glm library
set(GLM_INCLUDE ${THIRD_PARTY_DIR}/glm/include)

file(GLOB_RECURSE HEADER_FILES "*.h")
file(GLOB_RECURSE SOURCE_FILES "*.cc")

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${HEADER_FILES} ${SOURCE_FILES})

# add_library(${TARGET_NAME} ${HEADER_FILES} ${SOURCE_FILES} ${VOLK_SRC})
add_library(${TARGET_NAME} ${HEADER_FILES} ${SOURCE_FILES})
set_target_properties(${TARGET_NAME} PROPERTIES CXX_STANDARD 17)
set_target_properties(${TARGET_NAME} PROPERTIES FOLDER "Engine")

# link third party library
target_link_libraries(${TARGET_NAME} PUBLIC ${VULKAN_LIB})
target_link_libraries(${TARGET_NAME} PUBLIC spdlog::spdlog)
target_link_libraries(${TARGET_NAME} PUBLIC assimp)
target_link_libraries(${TARGET_NAME} PUBLIC tinyobjloader)
target_link_libraries(${TARGET_NAME} PUBLIC stb)
# target_link_libraries(${TARGET_NAME} PUBLIC imgui)
target_link_libraries(${TARGET_NAME} PUBLIC glfw)
target_link_libraries(${TARGET_NAME} PRIVATE $<BUILD_INTERFACE:json11>)

# include third party header
target_include_directories(
    ${TARGET_NAME}
    PUBLIC $<BUILD_INTERFACE:${VULKAN_INCLUDE}>
)

target_include_directories(
    ${TARGET_NAME}
    PUBLIC $<BUILD_INTERFACE:${JSON_INCLUDE}>
)

target_include_directories(
    ${TARGET_NAME}
    PUBLIC $<BUILD_INTERFACE:${GLM_INCLUDE}>
)

# target_include_directories(
#     ${TARGET_NAME}
#     PUBLIC $<BUILD_INTERFACE:${VOLK_INCLUDE}>
# )

target_include_directories(
    ${TARGET_NAME}
    PUBLIC $<BUILD_INTERFACE:${ENGINE_ROOT_DIR}/src>
    $<INSTALL_INTERFACE:include/${TARGET_NAME}>
)

target_include_directories(
    ${TARGET_NAME}
    PUBLIC $<BUILD_INTERFACE:${ENGINE_ROOT_DIR}/src/runtime>
    $<INSTALL_INTERFACE:include/${TARGET_NAME}>
)
