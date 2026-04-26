## handles adding external libraries

# TODO - disable external sample targets
# glm
add_subdirectory(glm)

# stb
add_subdirectory(stb)

# glad
add_subdirectory(glad)

# glfw
add_subdirectory(glfw)

# assimp
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)
add_subdirectory(assimp)

# tinyply
add_subdirectory(tinyply)

# imgui
add_subdirectory(imgui)

# implot
add_subdirectory(implot)

# glWrappers
add_subdirectory(glWrappers)

# TODO - consider adding GLSL language validator for validating shader code (https://github.com/KhronosGroup/glslang)
# TODO - add the following extensions to glad:
#	* bindless textures
#	* subgroup_ballot
#	* subgroup vote

## combining external libraries into one target
add_library(externalLibraries INTERFACE)

target_link_libraries(externalLibraries
	INTERFACE
		OpenGL::GL
		glm::glm
		stb
		glad
		glfw
		assimp
		tinyply
		imgui
		implot
		glWrappers
)
