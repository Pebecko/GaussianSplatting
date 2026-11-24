#include <filesystem>
#include <iostream>

#include "gaussianSplatting/glProgramSetup.hpp"

#include "openglResources.hpp"
#include "bufferStructures.hpp"


Programs::Programs() {
	std::string infoLog;

	// compute
	// assigns each splat to all of the screen tiles that it might be visible in
	this->assignSplatsToScreenTilesProgram = setupComputeProgram(std::filesystem::path(DEMO_SHADERS_DIRECTORY"/compute/assignSplatsToScreenTilesProgram.comp"));
	std::cout << this->assignSplatsToScreenTilesProgram.getInfoLog();

	// order all splats in each screen tile front to back for rendering using bitonic sort
	this->orderSplatsInScreenTilesSharedMemoryProgram = setupComputeProgram(std::filesystem::path(DEMO_SHADERS_DIRECTORY"/compute/orderSplatsInScreenTilesSharedMemoryProgram.comp"));
	std::cout << this->orderSplatsInScreenTilesSharedMemoryProgram.getInfoLog();

	// order all splats in each screen tile front to back for rendering using bitonic sort
	this->orderSplatsInScreenTilesGlobalMemoryProgram = setupComputeProgram(std::filesystem::path(DEMO_SHADERS_DIRECTORY"/compute/orderSplatsInScreenTilesGlobalMemoryProgram.comp"));
	std::cout << this->orderSplatsInScreenTilesGlobalMemoryProgram.getInfoLog();

	// rendering gaussian splats to texture
	this->renderTiledSplatsToTextureProgram = setupComputeProgram(std::filesystem::path(DEMO_SHADERS_DIRECTORY"/compute/renderTiledSplatsToTexture.comp"));
	std::cout << this->renderTiledSplatsToTextureProgram.getInfoLog();

	// rendering
	// debug xyz axes basis vector rendering rendering
	this->debugAxesRenderingProgram = setupRenderingProgram(
		std::filesystem::path(DEMO_SHADERS_DIRECTORY"/rendering/debugAxesRendering.vert"),
		std::filesystem::path(DEMO_SHADERS_DIRECTORY"/rendering/debugAxesRendering.frag")
	);
	std::cout << this->debugAxesRenderingProgram.getInfoLog();
}


Buffers::Buffers() {
	// TODO - consider combining some buffers
	// TODO - consider adding helper methods for data buffering

	// UBOs
	// single struct RenderingSettings
	this->renderingSettingsBuffer.allocateFixedSize(sizeof(RenderingSettings), GL_DYNAMIC_STORAGE_BIT);
	this->renderingSettingsBuffer.bindUniformBufferToBinding(RENDERING_SETTINGS_BUFFER_BINDING);
	
	// single struct ScreenTileSortingParameters
	this->screenTileSortingParametersBuffer.allocateFixedSize(sizeof(ScreenTileSortingParameters), GL_DYNAMIC_STORAGE_BIT);
	this->screenTileSortingParametersBuffer.bindUniformBufferToBinding(SCREEN_TILE_SORTING_PARAMETERS_BUFFER_BINDING);

	// SSBOs
	// array of structs GaussianSplat len MAX_GAUSSIAN_SPLATS
	this->gaussianSplatsBuffer.allocateFixedSize(MAX_GAUSSIAN_SPLATS * sizeof(GaussianSplat), GL_DYNAMIC_STORAGE_BIT);
	this->gaussianSplatsBuffer.bindShaderStorageBufferToBinding(GAUSSIAN_SPLATS_BUFFER_BINDING);

	// array of unsigned ints with the counts of gaussian splats assigned to each of the tiles
	this->screenTileSplatCountsBuffer.allocateFixedSize(MAX_SCREEN_TILES * sizeof(GLuint), GL_DYNAMIC_STORAGE_BIT);
	this->screenTileSplatCountsBuffer.bindShaderStorageBufferToBinding(SCREEN_TILE_SPLAT_COUNTS_BUFFER_BINDING);

	// array of structs ScreenTileSplatIndex len MAX_GAUSSIAN_SPLATS * MAX_SCREEN_TILES
	this->screenTileSplatIndicesFirstBuffer.allocateFixedSize((MAX_GAUSSIAN_SPLATS * MAX_SCREEN_TILES) * sizeof(ScreenTileSplatIndex), GL_DYNAMIC_STORAGE_BIT);
	this->screenTileSplatIndicesFirstBuffer.bindShaderStorageBufferToBinding(SCREEN_TILE_SPLAT_INDICES_INPUT_BUFFER_BINDING);
	// array of structs ScreenTileSplatIndex len MAX_GAUSSIAN_SPLATS * MAX_SCREEN_TILES
	this->screenTileSplatIndicesSecondBuffer.allocateFixedSize((MAX_GAUSSIAN_SPLATS * MAX_SCREEN_TILES) * sizeof(ScreenTileSplatIndex), GL_DYNAMIC_STORAGE_BIT);
	this->screenTileSplatIndicesSecondBuffer.bindShaderStorageBufferToBinding(SCREEN_TILE_SPLAT_INDICES_OUTPUT_BUFFER_BINDING);

	// empty VAO (rendering requires it even though it serves no actual purpose)
	//this->emptyVAO = gl::VertexArray();
}


Textures::Textures(GLint textureWidth, GLint textureHeight) {
	// could use the recreate() method of the implemented Texture class in the framebufferCallback() to alter the size of the texture according to the screen size
	// either make it the same as screen of round it up to some multiple of a power of (like 128)

	glCreateTextures(GL_TEXTURE_2D, 1, &this->renderingTexture);

// Allocate immutable storage
	glTextureStorage2D(this->renderingTexture, 1, GL_RGBA32F, textureWidth, textureHeight);

	glBindImageTexture(
		0,                  // binding = 0
		this->renderingTexture,
		0,                  // mip level
		GL_FALSE,
		0,
		GL_READ_WRITE,      // or GL_READ_WRITE
		GL_RGBA32F
	);

	glCreateFramebuffers(1, &this->framebuffer);

	glNamedFramebufferTexture(
		this->framebuffer,
		GL_COLOR_ATTACHMENT0,
		this->renderingTexture,
		0
	);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, this->framebuffer);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}


void Textures::blitToScreen(GLint screenWidth, GLint screenHeight) {
	glBlitFramebuffer(
		//0, 0, this->textureWidth, this->textureHeight,  // source dimension
		0, 0, screenWidth, screenHeight,  // source dimension
		0, 0, screenWidth, screenHeight,  // destination dimensions
		GL_COLOR_BUFFER_BIT,
		GL_NEAREST
	);
}
