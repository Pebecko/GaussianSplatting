#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "gaussianSplatting/primitives/gaussianSplat.hpp"
#include "gaussianSplatting/primitives/point.hpp"


// TODO - update in shaders
struct alignas(16) RenderingSettings {
	glm::mat4x4 projectionMatrix = glm::mat4x4(1.0f);
	glm::mat4x4 viewMatrix = glm::mat4x4(1.0f);

	GLfloat cameraFocalLength = 1.0f;
	GLfloat cameraPixelAspectRatio = 1.0f;
	glm::vec2 cameraPrincipalPointInPixels = glm::vec2(0.0f);

	GLfloat cameraNearDistance = 0.1f;  // TODO - synchronize these with camera settings somehow when free camera is added (or just pass those to the shaders)
	GLfloat cameraFarDistance = 128.0f;

	GLint screenWidth = 800;
	GLint screenHeight = 450;

	GLfloat basePointRadius = 128.0f;

	GLfloat _pad0[3] = {0.0f};

	RenderingSettings();

	GLfloat screenAspectRatio();

	glm::mat4x4 computeProjectionMatrix(
		GLfloat focalLength,  // in pixels?
		GLfloat pixelAspectRatio,
		//GLfloat skewAngleRadians,
		glm::vec2 principalPointPixels,
		GLuint imageWidthPixels,
		GLuint imageHeightPixels,
		GLfloat cameraNearDistance,
		GLfloat cameraFarDistance
	);

	glm::mat4x4 computeViewMatrix(
		glm::mat3x3 rotation,
		glm::vec3 translation
	);

	glm::mat3x2 computeProjectionJacobian(
		GLfloat focalLength,  // in pixels?
		GLfloat pixelAspectRatio,
		//GLfloat skewAngleRadians,
		glm::vec3 position
	);
};
static_assert(sizeof(RenderingSettings) == 176);
