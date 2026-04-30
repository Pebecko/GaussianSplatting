cmake_minimum_required(VERSION 3.16)
project(InfiniteSimulatedTerrainEngine)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# paths
# pass to the programs by setting up some compile definition?
set(SHADERS_DIR ${CMAKE_SOURCE_DIR}/shaders)

set(ASSETS_DIR ${CMAKE_SOURCE_DIR}/assets)
# set(MODELS_DIR ${ASSETS_DIR}/models)
# set(TEXTURES_DIR ${ASSETS_DIR}/textures)


add_compile_definitions(SHADERS_DIRECTORY="${SHADERS_DIR}")
# add_compile_definitions(MODELS_DIRECTORY="${MODELS_DIR}")
# add_compile_definitions(TEXTURES_DIRECTORY="${TEXTURES_DIR}")


# runtime output directory
# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)  # -> build/bin/{executable_name}.exe

# git submodules: https://git-scm.com/book/en/v2/Git-Tools-Submodules
# when cloning a repo with submodules two commands need to be ran to get the submodules:
#	git submodule init
#	git submodule update
# similar thing can be archieved directly when cloning the repo using --recurse-submodules
#	git clone --recurse-submodules https://...
# if a repo is already cloned but without the submodules the following command can be used to obtain them:
#	git submodule update --init --recursive

add_subdirectory(external)

file(GLOB_RECURSE GAUSSIAN_SPLATTING_SOURCE_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp"
)

add_library(gaussianSplatting STATIC ${GAUSSIAN_SPLATTING_SOURCE_FILES})

target_link_libraries(gaussianSplatting 
	PUBLIC
		externalLibraries
)

target_include_directories(gaussianSplatting
	PUBLIC
		"${CMAKE_CURRENT_SOURCE_DIR}/include"
)

add_subdirectory(demos)
