set(TARGET_NAME PeanutTests)

file(GLOB TEST_SOURCES CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/*.cc)

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" FILES ${TEST_SOURCES})
add_compile_definitions("PEANUT_ROOT_DIR=${PEANUT_ROOT_DIR}")

enable_testing()

add_executable(${TARGET_NAME} ${TEST_SOURCES})

set_target_properties(${TARGET_NAME} PROPERTIES CXX_STANDARD 17 OUTPUT_NAME "PeanutTests")
set_target_properties(${TARGET_NAME} PROPERTIES FOLDER "Engine")

target_compile_options(${TARGET_NAME} PUBLIC "$<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/WX->")

target_link_libraries(${TARGET_NAME} PRIVATE GTest::gtest_main PeanutRuntime)

include(GoogleTest)
gtest_discover_tests(${TARGET_NAME})