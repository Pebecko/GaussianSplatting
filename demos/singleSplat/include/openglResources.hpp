#pragma once

#include <glad/glad.h>

#include "glWrappers/buffer.hpp"
#include "glWrappers/vertexArray.hpp"
#include "glWrappers/shader.hpp"
#include "glWrappers/program.hpp"
#include "glWrappers/query.hpp"


// buffer bindings
#define RENDERING_SETTINGS_BUFFER_BINDING	0	// UBO

#define GAUSSIAN_SPLATS_BUFFER_BINDING		0	// SSBO


// parameters
#define MAX_GAUSSIAN_SPLATS 1


struct Programs {
	// compute
	//gl::Program orderAllSplatsProgram;
	//gl::Program orderSplatsInScreenTileProgram;
	//gl::Program assignSplatsToScreenTilesProgram;
	gl::Program renderAllSplatsToTextureProgram;

	// rendering
	//gl::Program debugPointRenderingProgram;
	//gl::Program debugEllipseRenderingProgram;
	gl::Program debugAxesRenderingProgram;

	Programs();
};


struct Buffers {
	gl::Buffer renderingSettingsBuffer;

	gl::Buffer gaussianSplatsBuffer;

	gl::VertexArray emptyVAO;

	Buffers();
};


struct Textures {
	GLuint renderingTexture;

	GLuint framebuffer;

	GLint textureWidth = 800;
	GLint textureHeight = 450;

	Textures(GLint textureWidth = 800, GLint textureHeight = 450);

	void blitToScreen(GLint screenWidth = 800, GLint screenHeight = 450);
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
