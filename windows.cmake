add_library(${PROJECT_NAME} SHARED
    src/image.cpp
    src/texture_data.cpp
    src/constants/texture_format.cpp
    ${basis_universal_SOURCE_DIR}/transcoder/basisu_transcoder.cpp
    ${basis_universal_SOURCE_DIR}/zstd/zstd.c
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    WINDOWS_EXPORT_ALL_SYMBOLS ON
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    NOMINMAX
)

target_include_directories(${PROJECT_NAME} PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/inc>"
    "${PROJECT_BINARY_DIR}"
)

target_include_directories(${PROJECT_NAME} PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${libwebp_SOURCE_DIR}/src"
    "${basis_universal_SOURCE_DIR}/transcoder"
)

target_link_libraries(${PROJECT_NAME}
    webpdecoder
)
