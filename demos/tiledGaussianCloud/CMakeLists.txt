file(GLOB_RECURSE TILED_GAUSSIAN_CLOUD_DEMO_SOURCE_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp"
)

add_executable(tiledGaussianCloud ${TILED_GAUSSIAN_CLOUD_DEMO_SOURCE_FILES})

target_link_libraries(tiledGaussianCloud
	gaussianSplatting
)

target_include_directories(tiledGaussianCloud
	PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_compile_definitions(tiledGaussianCloud
	PUBLIC
		DEMO_SHADERS_DIRECTORY="${CMAKE_CURRENT_SOURCE_DIR}/shaders"
)
