add_library(${PROJECT_NAME} SHARED
    src/image.cpp
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
)

target_link_libraries(${PROJECT_NAME}
    webpdecoder
)
