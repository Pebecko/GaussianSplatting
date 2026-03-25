add_library(imgui STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui_draw.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui_tables.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/imgui_widgets.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/backends/imgui_impl_glfw.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/backends/imgui_impl_opengl3.cpp
)

target_compile_definitions(imgui 
    PRIVATE
        IMGUI_IMPL_OPENGL_LOADER_GLAD
)

# target_compile_definitions(imgui PUBLIC IMGUI_DEFINE_MATH_OPERATORS)

target_link_libraries(imgui 
    PRIVATE
        glfw
        glad
)

target_include_directories(imgui
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/vendor  # figure out a way to make this PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)
