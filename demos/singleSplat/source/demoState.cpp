#include "demoState.hpp"


DemoState::DemoState() {
	this->rendering.gaussianSplats.reserve(MAX_GAUSSIAN_SPLATS);
	this->rendering.gaussianSplats.push_back(
		GaussianSplat(
			glm::vec3(0.0f),
			0.8f,
			glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
			glm::vec3(1.0f),
			SphericalHarmonics(glm::vec3(0.5f))
		)
	);
}

void DemoState::translateCamera(bool forward, bool left, bool back, bool right, bool up, bool down, float timeDelta) {
	glm::mat4 eyeMatrix = glm::inverse(this->rendering.renderingSettings.viewMatrix);

	// finding movement directions
	glm::vec3 forwardDirection = glm::vec3(
		eyeMatrix[2][0],
		eyeMatrix[2][1],
		eyeMatrix[2][2]
	);
	forwardDirection = glm::normalize(forwardDirection);

	glm::vec3 rightDirection = glm::vec3(
		eyeMatrix[0][0],
		eyeMatrix[0][1],
		eyeMatrix[0][2]
	);
	rightDirection = glm::normalize(rightDirection);

	/*glm::vec3 upDirection = glm::vec3(
		eyeMatrix[0][1],
		eyeMatrix[1][1],
		eyeMatrix[2][1]
	);*/
	glm::vec3 upDirection = glm::vec3(0.0f, 0.0f, 1.0f);
	upDirection = glm::normalize(upDirection);
	/*glm::vec3 rightDirection = glm::normalize(glm::cross(forwardDirection, upDirection));*/

	glm::vec3 translationVector = glm::vec3(0.0f);
	
	if (forward) {  // forwards (-z)
		translationVector -= forwardDirection;
	}
	if (back) {  // backwards (+z)
		translationVector += forwardDirection;
	}

	if (left) {  // left (-x)
		translationVector -= rightDirection;
	}
	if (right) {  // right (+x)
		translationVector += rightDirection;
	}

	if (up) {  // up (+y)
		translationVector -= upDirection;
	}
	if (down) {  // down (-y)
		translationVector += upDirection;
	}

	if (glm::dot(translationVector, translationVector) < 0.5f) {  // no change in position
		return;
	}

	// scaling the translation vector
	translationVector = glm::normalize(translationVector) * this->rendering.cameraMoveSpeed * timeDelta;

	// updating the position
	this->rendering.renderingSettings.viewMatrix = glm::inverse(glm::translate(glm::mat4(1.0f ), translationVector) * eyeMatrix);

	this->updateFlags.renderingSettingsChanged = true;
}

void DemoState::rotateCamera(double mouseDeltaX, double mouseDeltaY) {
	float angleDeltaX = mouseDeltaY * this->rendering.cameraMouseSensitivity;
	float angleDeltaZ = mouseDeltaX * this->rendering.cameraMouseSensitivity;

	glm::mat4 eyeMatrix = glm::inverse(this->rendering.renderingSettings.viewMatrix);

	glm::mat4 translateToOrigin = glm::translate(glm::mat4(1.0f), glm::vec3(
		-eyeMatrix[3][0], 
		-eyeMatrix[3][1], 
		-eyeMatrix[3][2])
	);
	glm::mat4 translateFromOrigin = glm::translate(glm::mat4(1.0f), glm::vec3(
		eyeMatrix[3][0], 
		eyeMatrix[3][1], 
		eyeMatrix[3][2])
	);

	// up/down rotation
	glm::mat4 xAxisRotation = glm::rotate(glm::mat4(1.0f), -angleDeltaX, glm::vec3(1.0f, 0.0f, 0.0f));

	// left/right rotation
	glm::mat4 zAxisRotation = glm::rotate(glm::mat4(1.0f), angleDeltaZ, glm::vec3(0.0f, 0.0f, 1.0f));

	this->rendering.renderingSettings.viewMatrix = glm::inverse(translateFromOrigin * zAxisRotation * translateToOrigin * eyeMatrix * xAxisRotation);

	this->updateFlags.renderingSettingsChanged = true;
}
