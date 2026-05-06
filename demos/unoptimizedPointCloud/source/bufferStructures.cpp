#include <random>

#include "bufferStructures.hpp"


RenderingSettings::RenderingSettings() {
    //this->viewMatrix = glm::lookAt(glm::vec3(0.0f), glm::vec3(0.0f, 0.0f, 10.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    //this->projectionMatrix = glm::perspective(glm::radians(90.0f), 16.0f / 9.0f, 0.1f, 128.0f);  // TODO - update the aspect ratio based on the window size
	this->viewMatrix = this->computeViewMatrix(glm::mat3x3(1.0f), glm::vec3(0.0f, 0.0f, -12.0f));
	this->projectionMatrix = this->computeProjectionMatrix(
		this->cameraFocalLength,
		this->cameraPixelAspectRatio,
		this->cameraPrincipalPointInPixels,
		this->screenWidth,
		this->screenHeight,
		this->cameraNearDistance,
		this->cameraFarDistance
	);
}


GLfloat RenderingSettings::screenAspectRatio() {
    return ((GLfloat) this->screenWidth) / ((GLfloat) this->screenHeight);
}


// https://songho.ca/opengl/gl_projectionmatrix.html
glm::mat4x4 RenderingSettings::computeProjectionMatrix(
	GLfloat focalLengthPixels,
	GLfloat pixelAspectRatio,
	//GLfloat skewAngleRadians,
	glm::vec2 principalPointPixels,
	GLuint imageWidthPixels,
	GLuint imageHeightPixels,
	GLfloat cameraNearDistance,
	GLfloat cameraFarDistance
) {
	// proj[0][0] = 2.0f * fx / w;
	// proj[1][1] = 2.0f * fy / h;
	   
	// proj[2][0] = 1.0f - 2.0f * cx / w;
	// proj[2][1] = 2.0f * cy / h - 1.0f;
	   
	// proj[2][2] = -(far + near) / (far - near);
	// proj[2][3] = -1.0f;
	   
	// proj[3][2] = -(2.0f * far * near) / (far - near);

	/*T const tanHalfFovy = tan(fovy / static_cast<T>(2));

	mat<4, 4, T, defaultp> Result(static_cast<T>(0));
	Result[0][0] = static_cast<T>(1) / (aspect * tanHalfFovy);
	Result[1][1] = static_cast<T>(1) / (tanHalfFovy);
	Result[2][2] = zFar / (zFar - zNear);
	Result[2][3] = static_cast<T>(1);
	Result[3][2] = -(zFar * zNear) / (zFar - zNear);*/

	// TODO - check this
	//return glm::mat4x4(
	//	2.0f * pixelAspectRatio * focalLengthPixels / imageWidthPixels, 0.0f, 0.0f, 0.0f,
	//	0.0f, 2.0f * focalLengthPixels / imageHeightPixels, 0.0f, 0.0f,
	//	1.0f - 2.0f * principalPointPixels.x / imageWidthPixels, 2.0f * principalPointPixels.y / imageHeightPixels - 1.0f, -(cameraFarDistance + cameraNearDistance) / (cameraFarDistance - cameraNearDistance), -1.0f,
	//	0.0f, 0.0f, -(2.0f * cameraFarDistance * cameraNearDistance) / (cameraFarDistance - cameraNearDistance), 0.0f
	//);

	glm::mat4 projectionMatrix(0.0f);

	float fx = pixelAspectRatio * focalLengthPixels;
	float fy = focalLengthPixels;

	float w = (float) imageWidthPixels;
	float h = (float) imageHeightPixels;
	float n = cameraNearDistance;
	float f = cameraFarDistance;

	// scale
	projectionMatrix[0][0] = 2.0f * fx / w;
	projectionMatrix[1][1] = 2.0f * fy / h;

	// principal point
	projectionMatrix[2][0] = 2.0f * principalPointPixels.x / w - 1.0f;
	projectionMatrix[2][1] = 1.0f - 2.0f * principalPointPixels.y / h;

	// depth
	projectionMatrix[2][2] = -(f + n) / (f - n);
	projectionMatrix[2][3] = -1.0f;

	projectionMatrix[3][2] = -(2.0f * f * n) / (f - n);

	return projectionMatrix;
}

glm::mat4x4 RenderingSettings::computeViewMatrix(
	glm::mat3x3 rotation,
	glm::vec3 translation
) {
	return glm::mat4x4(
		rotation[0][0], rotation[1][0], rotation[2][0], 0.0f,
		rotation[0][1], rotation[1][1], rotation[2][1], 0.0f,
		rotation[0][2], rotation[1][2], rotation[2][2], 0.0f,
		translation[0], translation[1], translation[2], 1.0f
	);
}

glm::mat3x2 RenderingSettings::computeProjectionJacobian(
	GLfloat focalLength,  // in pixels?
	GLfloat pixelAspectRatio,
	//GLfloat skewAngleRadians,
	glm::vec3 position
) {
	return glm::mat3x2(
		pixelAspectRatio * focalLength / position.z, 0.0f, (-pixelAspectRatio * focalLength * position.x) / (position.z * position.z),
		0.0f, focalLength / position.z, (-focalLength * position.y) / (position.z * position.z)
	);
}