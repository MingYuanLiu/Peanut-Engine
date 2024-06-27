set(ENGINE_ROOT_DIR "${PEANUT_ROOT_DIR}/engine")
set(THIRD_PARTY_DIR "${ENGINE_ROOT_DIR}/3rdparty")

# Windows compiler
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    add_compile_options("/MP")
endif()

set(VULKAN_INCLUDE ${THIRD_PARTY_DIR}/VulkanSDK/include)
add_compile_definitions("ENABLE_VULKAN=1")

if(WIN32)
    set(VULKAN_LIB ${THIRD_PARTY_DIR}/VulkanSDK/lib/Win32/vulkan-1.lib)
    set(GLSLANGVALIDATOR_EXECUTABLE ${THIRD_PARTY_DIR}/VulkanSDK/bin/Win32/glslangValidator.exe)
    add_compile_definitions("PEANUT_VK_LAYER_PATH=${THIRD_PARTY_DIR}/VulkanSDK/bin/Win32")
    add_compile_definitions("PE_PLATFORM_WINDOWS=1")
    add_compile_definitions("VK_USE_PLATFORM_WIN32_KHR=1")
    # use volk
    # add_compile_definitions("VK_NO_PROTOTYPES=1")
    set(VULKAN_FOUND true)
elseif(UNIX)
    set(VULKAN_LIB ${THIRD_PARTY_DIR}/VulkanSDK/lib/Linux/libvulkan.so.1)
    set(GLSLANGVALIDATOR_EXECUTABLE ${THIRD_PARTY_DIR}/VulkanSDK/bin/Linux/glslangValidator)
    add_compile_definitions("PEANUT_VK_LAYER_PATH=${THIRD_PARTY_DIR}/VulkanSDK/bin/Linux")
    add_compile_definitions("PE_PLATFORM_UNIX=1")
    # use volk
    # add_compile_definitions("VK_NO_PROTOTYPES=1")
    add_compile_definitions("VK_USE_PLATFORM_XLIB_KHR=1")
    set(VULKAN_FOUND true)
else()
    message(FATAL_ERROR "Unknow Platform")
endif()

set(CMAKE_BUILD_TYPE "Debug")
# enable glm experimental 
add_compile_definitions("GLM_ENABLE_EXPERIMENTAL")

add_subdirectory(src/shaders)
add_subdirectory(3rdparty)
add_subdirectory(src/runtime)
add_subdirectory(src/test)
add_subdirectory(src/reflection_parser)

# add_dependencies(PeanutRuntime EngineShader)
