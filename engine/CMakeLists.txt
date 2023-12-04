set(ENGINE_ROOT_DIR "${PEANUT_ROOT_DIR}/engine")
set(THIRD_PARTY_DIR "${ENGINE_ROOT_DIR}/3rdparty")

# Windows compiler
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    add_compile_options("/MP")
endif()

set(VULKAN_INCLUDE ${THIRD_PARTY_DIR}/VulkanSDK/include)

if(WIN32)
    set(VULKAN_LIB ${THIRD_PARTY_DIR}/VulkanSDK/lib/Win32/vulkan-1.lib)
    set(GLSLANGVALIDATOR_EXECUTABLE ${THIRD_PARTY_DIR}/VulkanSDK/bin/Win32/glslangValidator.exe)
    add_compile_definitions("PEANUT_VK_LAYER_PATH=${THIRD_PARTY_DIR}/VulkanSDK/bin/Win32")
    add_compile_definitions("PE_PLATFORM_WINDOWS=1")
elseif(UNIX)
    set(VULKAN_LIB ${THIRD_PARTY_DIR}/VulkanSDK/lib/Linux/libvulkan.so.1)
    set(GLSLANGVALIDATOR_EXECUTABLE ${THIRD_PARTY_DIR}/VulkanSDK/bin/Linux/glslangValidator)
    add_compile_definitions("PEANUT_VK_LAYER_PATH=${THIRD_PARTY_DIR}/VulkanSDK/bin/Linux")
    add_compile_definitions("PE_PLATFORM_UNIX=1")
else()
    message(FATAL_ERROR "Unknow Platform")
endif()

add_subdirectory(3rdparty)
add_subdirectory(src/runtime)
add_subdirectory(src/test)
