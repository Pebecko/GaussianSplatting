#pragma once

#include <glad/glad.h>

#include "glWrappers/buffer.hpp"
#include "glWrappers/vertexArray.hpp"
#include "glWrappers/shader.hpp"
#include "glWrappers/program.hpp"
#include "glWrappers/query.hpp"


// buffer bindings
#define RENDERING_SETTINGS_BUFFER_BINDING				0	// UBO
#define SCREEN_TILE_SORTING_PARAMETERS_BUFFER_BINDING	1	// UBO

#define GAUSSIAN_SPLATS_BUFFER_BINDING					0	// SSBO
#define SCREEN_TILE_SPLAT_COUNTS_BUFFER_BINDING			1	// SSBO
#define SCREEN_TILE_SPLAT_INDICES_INPUT_BUFFER_BINDING	2	// SSBO
#define SCREEN_TILE_SPLAT_INDICES_OUTPUT_BUFFER_BINDING	3	// SSBO


// parameters
#define MAX_GAUSSIAN_SPLATS 16384

#define MAX_SCREEN_TILE_ROWS 16
#define MAX_SCREEN_TILE_COLUMNS 16
#define MAX_SCREEN_TILES (MAX_SCREEN_TILE_COLUMNS * MAX_SCREEN_TILE_ROWS)


struct Programs {
	// compute
	//gl::Program orderAllSplatsProgram;
	gl::Program assignSplatsToScreenTilesProgram;
	gl::Program orderSplatsInScreenTilesSharedMemoryProgram;
	gl::Program orderSplatsInScreenTilesGlobalMemoryProgram;
	gl::Program renderTiledSplatsToTextureProgram;

	// rendering
	//gl::Program debugPointRenderingProgram;
	//gl::Program debugEllipseRenderingProgram;
	gl::Program debugAxesRenderingProgram;

	Programs();
};


struct Buffers {
	gl::Buffer renderingSettingsBuffer;
	gl::Buffer screenTileSortingParametersBuffer;

	gl::Buffer gaussianSplatsBuffer;
	gl::Buffer screenTileSplatCountsBuffer;
	gl::Buffer screenTileSplatIndicesFirstBuffer;
	gl::Buffer screenTileSplatIndicesSecondBuffer;

	gl::VertexArray emptyVAO;

	Buffers();
};


struct Textures {
	GLuint renderingTexture;

	GLuint framebuffer;

	// TODO - make this size adaptable
	GLint textureWidth = 2000;
	GLint textureHeight = 2000;

	Textures(GLint textureWidth = 2000, GLint textureHeight = 2000);

	void blitToScreen(GLint screenWidth = 2000, GLint screenHeight = 2000);
};


struct Queries {
	//gl::BeginEndQuery<GL_TIME_ELAPSED> dataBufferingTimeElapsedQuery;

	gl::BeginEndQuery<GL_TIME_ELAPSED> mainRenderingTimeElapsedQuery;
};


struct OpenGLResources {
	Programs programs;
	Buffers buffers;
	Textures textures;
	Queries queries;

	OpenGLResources() = default;
};
