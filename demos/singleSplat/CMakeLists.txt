add_executable(
	singleSplat

)

add_executable(clusteredForwardShading "source/main.cpp" "source/openglResources.cpp" "source/demoState.cpp"  "source/bufferStructures.cpp"  "source/clusteredForwardShading.cpp")

file(GLOB_RECURSE SINGLE_SPLAT_DEMO_SOURCE_FILES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp
)

target_link_libraries(clusteredForwardShading
	externalLibraries
	iste
	demoFramework
)

target_include_directories(clusteredForwardShading
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_definitions(clusteredForwardShading 
	PRIVATE
		 SHADER_SOURCE_DIRECTORY=\"${CMAKE_CURRENT_SOURCE_DIR}/shaders\"
)
