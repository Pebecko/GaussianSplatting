file(GLOB_RECURSE SINGLE_SPLAT_DEMO_SOURCE_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp"
)

add_executable(singleSplat ${SINGLE_SPLAT_DEMO_SOURCE_FILES})

target_link_libraries(singleSplat
	gaussianSplatting
)

target_include_directories(singleSplat
	PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}/include"
)
