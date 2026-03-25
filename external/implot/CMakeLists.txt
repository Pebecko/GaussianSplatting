add_library(implot STATIC
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/implot.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/implot_items.cpp
)

target_link_libraries(implot 
    PRIVATE
        imgui
)

target_include_directories(implot
    PUBLIC
        # ${CMAKE_CURRENT_SOURCE_DIR}/vendor  # figure out a way to make this PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)
