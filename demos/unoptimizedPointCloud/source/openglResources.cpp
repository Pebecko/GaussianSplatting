#include <filesystem>
#include <iostream>

#include "gaussianSplatting/glProgramSetup.hpp"

#include "openglResources.hpp"
#include "bufferStructures.hpp"


Programs::Programs() {
	std::string infoLog;

	// compute
	// updating light positions
	this->renderAllSplatsToTextureProgram = setupComputeProgram(std::filesystem::path(SHADERS_DIRECTORY"/compute/renderAllSplatsToTexture.comp"));
	std::cout << this->renderAllSplatsToTextureProgram.getInfoLog();

	// rendering
	// debug xyz axes basis vector rendering rendering
	this->debugAxesRenderingProgram = setupRenderingProgram(
		std::filesystem::path(SHADERS_DIRECTORY"/rendering/debugAxesRendering.vert"),
		std::filesystem::path(SHADERS_DIRECTORY"/rendering/debugAxesRendering.frag")
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

	// SSBOs
	// array of structs GaussianSplat len MAX_GAUSSIAN_SPLATS
	this->gaussianSplatsBuffer.allocateFixedSize(MAX_GAUSSIAN_SPLATS * sizeof(GaussianSplat), GL_DYNAMIC_STORAGE_BIT);
	this->gaussianSplatsBuffer.bindShaderStorageBufferToBinding(GAUSSIAN_SPLATS_BUFFER_BINDING);

	// empty VAO (rendering requires it even though it serves no actual purpose)
	//this->emptyVAO = gl::VertexArray();

	// // vertices
	// this->meshVAO.bindVertexBuffer(this->meshVertexBuffer, 0u, 0u, sizeof(...));
	// this->meshVAO.enableAttribute(0u);
	// this->meshVAO.mapAttributeToBinding(0u, 0u);
	// this->meshVAO.setFloatAttributeFormat(0u, 3, GL_FLOAT, GL_FALSE, 0u);
	// // normals
	// this->meshVAO.bindVertexBuffer(this->meshNormalBuffer, 1u, 0u, sizeof(...));
	// this->meshVAO.mapAttributeToBinding(1u, 1u);
	// this->meshVAO.enableAttribute(1u);
	// this->meshVAO.setFloatAttributeFormat(0u, 3, GL_FLOAT, GL_FALSE, 0u);
	// this->meshVAO.bindingInstancedAdvanceDivisor(1u, 2u);  // normals advance once every 3 vertices (identical for each triangle)
}


Textures::Textures(GLint textureWidth, GLint textureHeight) {
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
