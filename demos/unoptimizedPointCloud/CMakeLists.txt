file(GLOB_RECURSE UNOPTIMIZED_GAUSSIAN_CLOUD_DEMO_SOURCE_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp"
)

add_executable(unoptimizedGaussianCloud ${UNOPTIMIZED_GAUSSIAN_CLOUD_DEMO_SOURCE_FILES})

target_link_libraries(unoptimizedGaussianCloud
	gaussianSplatting
)

target_include_directories(unoptimizedGaussianCloud
	PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}/include"
)
