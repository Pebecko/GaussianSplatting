#pragma once

#include <string_view>
#include <vector>

//#include <glfw/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "bufferStructures.hpp"
#include "openglResources.hpp"


//#define WORK_GROUP_SIZE_X 4
//#define WORK_GROUP_SIZE_Y 4
//#define WORK_GROUP_SIZE_Z 4


enum class GenerationBackend {
	GPU_SINGLE_WORKGROUP
};

enum class RenderingMode {
	ALL_GAUSSIAN_SPLATS
};

struct Timing {
	GLuint64 dataBufferingMs = 0ul;

	GLuint64 gaussianSplatsClusteringMs = 0ul;
	GLuint64 gaussianSplatsOrderingMs = 0ul;
	GLuint64 gaussianSplatsRenderingMs = 0ul;
};


struct UpdateFlags {
	bool recompileShaders = false;

	bool gaussianSplatsChanged = true;

	bool renderingSettingsChanged = true;
};


enum class CameraMode {
	FREE_CAMERA = 0,  // mouse and keyboard control the camera movement
	STATIC_CAMERA = 1,  // camera stays stationary at its previous position and keyboard and mouse control the gui
	// CAMERA_MODE_FIXED_CAMERA  // camera is fixed in place in one of the predetermined positions
	TOTAL_MODES
};

struct RenderingState {
	RenderingSettings renderingSettings;

	bool uniformPointSize = false;

	RenderingMode renderingMode = RenderingMode::ALL_GAUSSIAN_SPLATS;
	CameraMode cameraMode = CameraMode::STATIC_CAMERA;

	GLfloat cameraFovAngleY = 90.0f;

	GLfloat cameraMoveSpeed = 5.0f;
	GLfloat cameraMouseSensitivity = 0.01f;

	//double currentMouseX = 0.0;
	//double currentMouseY = 0.0;

	double previousMouseX = 0.0;
	double previousMouseY = 0.0;

	bool renderGui = true;
	bool renderAxes = true;

	std::vector<GaussianSplat> gaussianSplats;
};


struct DemoState {
	RenderingState rendering;

	Timing timings;

	UpdateFlags updateFlags;

	DemoState();

	void translateCamera(bool forward, bool left, bool back, bool right, bool up, bool down, float timeDelta);  // call once every frame when camera is in free mode
	void rotateCamera(double mouseDeltaX, double mouseDeltaY);

	void appendGaussianSplat(GaussianSplat gaussianSplat);
};
