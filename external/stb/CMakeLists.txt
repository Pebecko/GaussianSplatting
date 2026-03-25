add_library(stb STATIC
	${CMAKE_CURRENT_SOURCE_DIR}/stbImplementations.cpp
)

target_include_directories(stb 
	PUBLIC 
		${CMAKE_CURRENT_SOURCE_DIR}/include

)