cmake_minimum_required(VERSION 3.22.1)

include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.17.0
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# ---------------------------------------------------------------------------
# Universal tests — no GPU required, links against campello_image.
# ---------------------------------------------------------------------------
add_executable(campello_image_universal_tests
    universal/test_image.cpp
)

target_include_directories(campello_image_universal_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/inc
    ${CMAKE_BINARY_DIR}
)

target_link_libraries(campello_image_universal_tests
    campello_image
    GTest::gtest_main
)

target_compile_definitions(campello_image_universal_tests PRIVATE
    CAMPELLO_IMAGE_TEST_ASSETS_DIR="${CMAKE_CURRENT_SOURCE_DIR}/images"
)

set_target_properties(campello_image_universal_tests PROPERTIES
    CXX_STANDARD 20
)

# On Windows, campello_image is a DLL. Copy it next to the test binary so
# CTest can find it when running test discovery and execution.
if(WIN32)
    add_custom_command(TARGET campello_image_universal_tests POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:campello_image>
            $<TARGET_FILE_DIR:campello_image_universal_tests>
    )
endif()

include(GoogleTest)
gtest_discover_tests(campello_image_universal_tests)
