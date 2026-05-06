#include <iostream>
#include <filesystem>

#include "glad/glad.h"
#include "glfw/glfw3.h"
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl3.h"

#include "glWrappers/debug.hpp"

#include "gaussianSplatting/glProgramSetup.hpp"
#include "gaussianSplatting/imguiWrapper.hpp"

#include "openglResources.hpp"
#include "demoState.hpp"


#define CAMERA_MODE_SWITCH_KEY GLFW_KEY_F1
#define TOGGLE_GUI_KEY GLFW_KEY_F2

#define CAMERA_MOVE_FORWARDS_KEY	GLFW_KEY_W
#define CAMERA_MOVE_LEFT_KEY		GLFW_KEY_A
#define CAMERA_MOVE_BACKWARDS_KEY	GLFW_KEY_S
#define CAMERA_MOVE_RIGHT_KEY		GLFW_KEY_D
#define CAMERA_MOVE_UP_KEY			GLFW_KEY_SPACE
#define CAMERA_MOVE_DOWN_KEY		GLFW_KEY_LEFT_SHIFT


// TODO - instead of free cam use mouse to rotate the camera and mouse wheel to zoom in/out with camera centered to the center of the gaussian

void errorCallback(int error, const char* description) {
	std::cerr << "===============" << std::endl;
	std::cerr << "GLFW Error (" << error << "): " << description << std::endl;
	std::cerr << "===============" << std::endl;
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);

	DemoState* state = (DemoState*) glfwGetWindowUserPointer(window);

	if (state == nullptr) {
		return;
	}

	if (width != 0 && height != 0) {
		state->rendering.renderingSettings.screenWidth = width;
		state->rendering.renderingSettings.screenHeight = height;
		state->rendering.renderingSettings.cameraPrincipalPointInPixels = glm::vec2(static_cast<float>(width) / 2.0f, static_cast<float>(height) / 2.0f);

		//state->rendering.renderingSettings.projectionMatrix = glm::perspective(
		//	glm::radians(state->rendering.cameraFovAngleY),
		//	state->rendering.renderingSettings.screenAspectRatio(),
		//	state->rendering.renderingSettings.cameraNearDistance,
		//	state->rendering.renderingSettings.cameraFarDistance
		//);
		state->rendering.renderingSettings.projectionMatrix = state->rendering.renderingSettings.computeProjectionMatrix(
			state->rendering.renderingSettings.cameraFocalLength,
			state->rendering.renderingSettings.cameraPixelAspectRatio,
			state->rendering.renderingSettings.cameraPrincipalPointInPixels,
			state->rendering.renderingSettings.screenWidth,
			state->rendering.renderingSettings.screenHeight,
			state->rendering.renderingSettings.cameraNearDistance,
			state->rendering.renderingSettings.cameraFarDistance
		);

		state->updateFlags.renderingSettingsChanged = true;
	}
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	DemoState* state = (DemoState*) glfwGetWindowUserPointer(window);

	if (state == nullptr) {
		return;
	}

	ImGuiIO& io = ImGui::GetIO();
	if (io.WantCaptureKeyboard) {
		return;
	}

	if (key == CAMERA_MODE_SWITCH_KEY && action == GLFW_PRESS) {
		state->rendering.cameraMode = CameraMode((int(state->rendering.cameraMode) + 1) % int(CameraMode::TOTAL_MODES));

		if (state->rendering.cameraMode == CameraMode::FREE_CAMERA) {
			glfwSetCursorPos(window, state->rendering.renderingSettings.screenWidth * 0.5, state->rendering.renderingSettings.screenHeight * 0.5);
		}
	}
	if (key == TOGGLE_GUI_KEY && action == GLFW_PRESS) {
		state->rendering.renderGui = !state->rendering.renderGui;
	}
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
	DemoState* state = (DemoState*) glfwGetWindowUserPointer(window);

	if (state == nullptr) {
		return;
	}

	ImGuiIO& io = ImGui::GetIO();
	if (io.WantCaptureMouse) {
		return;
	}

	// actions
}

void cursorCallback(GLFWwindow* window, double x, double y) {
	DemoState* state = (DemoState*) glfwGetWindowUserPointer(window);

	if (state == nullptr) {
		return;
	}

	ImGuiIO& io = ImGui::GetIO();
	if (io.WantCaptureMouse) {
		return;
	}

	double mouseDeltaX = x - state->rendering.previousMouseX;
	double mouseDeltaY = y - state->rendering.previousMouseY;

	state->rendering.previousMouseX = x;
	state->rendering.previousMouseY = y;

	if (state->rendering.cameraMode == CameraMode::FREE_CAMERA) {
		state->rotateCamera(mouseDeltaX, mouseDeltaY);
		//glfwSetCursorPos(window, state->rendering.renderingSettings.screenWidth * 0.5, state->rendering.renderingSettings.screenHeight * 0.5);
	}

	//state->rendering.previousMouseX = state->rendering.currentMouseX;
	//state->rendering.previousMouseY = state->rendering.currentMouseY;

	//state->rendering.currentMouseX = x;
	//state->rendering.currentMouseY = y;
}


GLFWwindow* initializeApplication() {
	// TODO - move setup and cleanup into separate files and functions
	// GLFW setup https://www.glfw.org/docs/latest/quick.html
	if (glfwInit() == GLFW_FALSE) {
		// GLFW init failed
		printf("GLFW initialization failed\n");
		exit(EXIT_FAILURE);
	}
	glfwSetErrorCallback(errorCallback);

	// setting minimum OpenGL version
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	// setting OpenGL profile
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);

	// creating a window and setting up context using GLFW
	GLFWwindow* window = glfwCreateWindow(1000, 600, "Single Gaussian Splat Demo", NULL, NULL);
	if (window == NULL) {
		// Window or OpenGL context creation failed
		exit(EXIT_FAILURE);
	}

	// current OpenGL context has to be selected before using OpenGL API
	// remains selected until another one is selected or the window is destroyed
	glfwMakeContextCurrent(window);

	gladLoadGL();  // initializing OpenGL extension loader library (glad) because the loader needs current context to load from

	// initializing OpenGL debugging
	gl::debug::initialize();

	// setting initial viewport size
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);

	// face culling (https://wikis.khronos.org/opengl/Face_Culling)
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	// blending (https://wikis.khronos.org/opengl/Blending)
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glEnable(GL_BLEND);

	// depth test (https://wikis.khronos.org/opengl/Depth_Test)
	glDepthFunc(GL_LESS);
	glEnable(GL_DEPTH_TEST);

	glClearColor(0.9f, 0.6f, 0.9f, 1.0f);

	glfwSetKeyCallback(window, keyCallback);
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
	glfwSetMouseButtonCallback(window, mouseButtonCallback);
	glfwSetCursorPosCallback(window, cursorCallback);

	// setting up ImGui
	demos::gui::init(window, "#version 460");

	ImGuiIO& io = ImGui::GetIO();

	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // enable keyboard controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;  // enable docking

	// io.IniFilename = nullptr;  // turns off ui persistence

	ImGui::StyleColorsDark();

	return window;
}

void updateBuffers(DemoState& state, OpenGLResources& glResources) {
	if (state.updateFlags.renderingSettingsChanged) {
		glResources.buffers.renderingSettingsBuffer.setData(state.rendering.renderingSettings);

		glPointSize(state.rendering.renderingSettings.basePointRadius);
		if (state.rendering.uniformPointSize) {
			glDisable(GL_PROGRAM_POINT_SIZE);
		}
		else {
			glEnable(GL_PROGRAM_POINT_SIZE);
		}

		state.updateFlags.renderingSettingsChanged = false;
	}

	if (state.updateFlags.gaussianSplatsChanged) {
		std::span gaussianSplats = state.rendering.gaussianSplats;
		glResources.buffers.gaussianSplatsBuffer.setArrayData(gaussianSplats);
		state.updateFlags.gaussianSplatsChanged = false;
	}

	glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);
}

void drawAxes(OpenGLResources& glResources) {
	// use glLineWidth() to change the line width
	glResources.buffers.emptyVAO.bind();
	glResources.programs.debugAxesRenderingProgram.use();
	// each axis is composed of a line segment (2 points) and there are 3 axes
	glDrawArraysInstanced(GL_LINES, 0, 2, 3);
}

void drawAllGaussians(DemoState& state, OpenGLResources& glResources) {
	glResources.programs.renderAllSplatsToTextureProgram.use();

	//glResources.queries.mainRenderingTimeElapsedQuery.begin();
	glDispatchCompute(
		(glResources.textures.textureWidth + 15) / 16,
		(glResources.textures.textureHeight + 15) / 16,
		1
	);
	//glResources.queries.mainRenderingTimeElapsedQuery.end();
	glResources.textures.blitToScreen(state.rendering.renderingSettings.screenWidth, state.rendering.renderingSettings.screenHeight);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}


void specifyGuiFrame(DemoState& state) {
	ImGui::Begin("Single Splat Demo - Settings");

	ImGuiIO& io = ImGui::GetIO();

	if (ImGui::Button("Recompile Shaders")) {
		state.updateFlags.recompileShaders = true;
	}

	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f * io.DeltaTime, io.Framerate);

	if (ImGui::CollapsingHeader("Gaussian Splat Properties", ImGuiTreeNodeFlags_DefaultOpen)) {

		// if (ImGui::SliderFloat3("Positions", glm::value_ptr(...), -10.0f, 10.0f)) {
		// 	state.updateFlags.gaussianSplatsChanged = true;
		// }

		if (ImGui::SliderFloat("Opacity", &state.rendering.gaussianSplats[0].opacity, 0.0f, 1.0f)) {
			state.updateFlags.gaussianSplatsChanged = true;
		}

		// if (ImGui::SliderFloat3("Covariance Rotation - Euler Angles", &..., -glm::pi<float>, glm::pi<float>)) {
		// 	state.updateFlags.gaussianSplatsChanged = true;
		// }

		if (ImGui::SliderFloat3("Covariance Scale - Standard Deviations", glm::value_ptr(state.rendering.gaussianSplats[0].covarianceScale), 0.001f, 10.0f)) {
			state.updateFlags.gaussianSplatsChanged = true;
		}

		if (ImGui::CollapsingHeader("Spherical Harmonics Coefficients", ImGuiTreeNodeFlags_DefaultOpen)) {
			if (ImGui::SliderFloat3("Order 0 - Coefficient 0", glm::value_ptr(state.rendering.gaussianSplats[0].sphericalHarmonics.coefficients[0]), 0.0f, 2.0f)) {
				state.updateFlags.gaussianSplatsChanged = true;
			}
			if (ImGui::SliderFloat3("Order 1 - Coefficient 0", glm::value_ptr(state.rendering.gaussianSplats[0].sphericalHarmonics.coefficients[1]), 0.0f, 1.0f)) {
				state.updateFlags.gaussianSplatsChanged = true;
			}
			if (ImGui::SliderFloat3("Order 1 - Coefficient 1", glm::value_ptr(state.rendering.gaussianSplats[0].sphericalHarmonics.coefficients[2]), 0.0f, 1.0f)) {
				state.updateFlags.gaussianSplatsChanged = true;
			}
			if (ImGui::SliderFloat3("Order 1 - Coefficient 2", glm::value_ptr(state.rendering.gaussianSplats[0].sphericalHarmonics.coefficients[3]), 0.0f, 1.0f)) {
				state.updateFlags.gaussianSplatsChanged = true;
			}
			if (ImGui::SliderFloat3("Order 2 - Coefficient 0", glm::value_ptr(state.rendering.gaussianSplats[0].sphericalHarmonics.coefficients[4]), 0.0f, 1.0f)) {
				state.updateFlags.gaussianSplatsChanged = true;
			}
			if (ImGui::SliderFloat3("Order 2 - Coefficient 1", glm::value_ptr(state.rendering.gaussianSplats[0].sphericalHarmonics.coefficients[5]), 0.0f, 1.0f)) {
				state.updateFlags.gaussianSplatsChanged = true;
			}
			if (ImGui::SliderFloat3("Order 2 - Coefficient 2", glm::value_ptr(state.rendering.gaussianSplats[0].sphericalHarmonics.coefficients[6]), 0.0f, 1.0f)) {
				state.updateFlags.gaussianSplatsChanged = true;
			}
			if (ImGui::SliderFloat3("Order 2 - Coefficient 3", glm::value_ptr(state.rendering.gaussianSplats[0].sphericalHarmonics.coefficients[7]), 0.0f, 1.0f)) {
				state.updateFlags.gaussianSplatsChanged = true;
			}
			if (ImGui::SliderFloat3("Order 2 - Coefficient 4", glm::value_ptr(state.rendering.gaussianSplats[0].sphericalHarmonics.coefficients[8]), 0.0f, 1.0f)) {
				state.updateFlags.gaussianSplatsChanged = true;
			}
		}
	}

	// could later have some submenu for keybinds
	//if (ImGui::CollapsingHeader("Keybinds", ImGuiTreeNodeFlags_DefaultOpen)) {
	//	ImGui::Text("F1 - switch between camera modes");
	//	ImGui::Text("F2 - toggle GUI visibility");
	//	ImGui::Text("W, A, S, D, Space, LShift - camera control");
	//}

	// TODO - application timing submenu
	//if (ImGui::CollapsingHeader("Time Measurements", ImGuiTreeNodeFlags_DefaultOpen)) {
	//	// data buffering timing
	//	// could use it both for general buffer updates and specifically when comparing CPU generation + data buffering
	//	// rendering timing
	//	ImGui::Text(
	//		"Total RENDERING time: %lu ns",
	//		state.timings.mainRenderingMs + state.timings.spotLightsDebugRenderingMs + state.timings.pointLightsDebugRenderingMs
	//	);
	//	ImGui::Text(
	//		"  Main rendering: %lu ns, Spot lights debug: %lu ns, Point lights debug: %lu ns",
	//		state.timings.mainRenderingMs,
	//		state.timings.spotLightsDebugRenderingMs,
	//		state.timings.pointLightsDebugRenderingMs
	//	);
	//}

	// rendering settings submenu
	static const char* renderingModeNames[] = {
		"Scalar Field",				// SCALAR_FIELD
		"CHUNK_CLUSTER_INDICES_DEBUG",	// CHUNK_CLUSTER_INDICES_DEBUG
		"CHUNK_CLUSTER_OCCUPANCY_DEBUG",	// CHUNK_CLUSTER_OCCUPANCY_DEBUG
		"Chunk Mesh NAIVE", 		// CHUNK_MESH_NAIVE
		"Chunk Mesh CLUSTERED",		// CHUCK_MESH_CLUSTERED
		"Model Mesh NAIVE", 		// MODEL_MESH_NAIVE
		"Model Mesh CLUSTERED",		// MODEL_MESH_CLUSTERED
	};
	int renderingMode = int(state.rendering.renderingMode);

	if (ImGui::CollapsingHeader("Rendering Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
		if (ImGui::SliderFloat("Camera Focal Length", &state.rendering.renderingSettings.cameraFocalLength, 16.0f, 4086.0f)) {
			state.rendering.renderingSettings.projectionMatrix = state.rendering.renderingSettings.computeProjectionMatrix(
				state.rendering.renderingSettings.cameraFocalLength,
				state.rendering.renderingSettings.cameraPixelAspectRatio,
				state.rendering.renderingSettings.cameraPrincipalPointInPixels,
				state.rendering.renderingSettings.screenWidth,
				state.rendering.renderingSettings.screenHeight,
				state.rendering.renderingSettings.cameraNearDistance,
				state.rendering.renderingSettings.cameraFarDistance
			);
			state.updateFlags.renderingSettingsChanged = true;
		}
		if (ImGui::SliderFloat("Camera Pixel Aspect Ratio", &state.rendering.renderingSettings.cameraPixelAspectRatio, 0.1f, 10.0f)) {
			state.rendering.renderingSettings.projectionMatrix = state.rendering.renderingSettings.computeProjectionMatrix(
				state.rendering.renderingSettings.cameraFocalLength,
				state.rendering.renderingSettings.cameraPixelAspectRatio,
				state.rendering.renderingSettings.cameraPrincipalPointInPixels,
				state.rendering.renderingSettings.screenWidth,
				state.rendering.renderingSettings.screenHeight,
				state.rendering.renderingSettings.cameraNearDistance,
				state.rendering.renderingSettings.cameraFarDistance
			);
			state.updateFlags.renderingSettingsChanged = true;
		}
		if (ImGui::SliderFloat2("Camera Principal Point", glm::value_ptr(state.rendering.renderingSettings.cameraPrincipalPointInPixels), 0.0f, float(state.rendering.renderingSettings.screenWidth))) {
			state.rendering.renderingSettings.projectionMatrix = state.rendering.renderingSettings.computeProjectionMatrix(
				state.rendering.renderingSettings.cameraFocalLength,
				state.rendering.renderingSettings.cameraPixelAspectRatio,
				state.rendering.renderingSettings.cameraPrincipalPointInPixels,
				state.rendering.renderingSettings.screenWidth,
				state.rendering.renderingSettings.screenHeight,
				state.rendering.renderingSettings.cameraNearDistance,
				state.rendering.renderingSettings.cameraFarDistance
			);
			state.updateFlags.renderingSettingsChanged = true;
		}

		// dropdown for selecting rendering mode
		//if (ImGui::Combo("Rendering Mode", &renderingMode, renderingModeNames, IM_ARRAYSIZE(renderingModeNames))) {
		//	state.rendering.renderingMode = RenderingMode(renderingMode);
		//}

		// checkbox for uniform point size
		//if (ImGui::Checkbox("Uniform Point Size", &state.rendering.uniformPointSize)) {
		//	state.updateFlags.renderingSettingsChanged = true;
		//}
		// slider for basePointRadius
		//if (ImGui::SliderFloat("Base Point Radius", &state.rendering.renderingSettings.basePointRadius, 1.0f, 256.0f)) {
		//	state.updateFlags.renderingSettingsChanged = true;
		//}

		//// camera Y FOV angle
		//if (ImGui::SliderFloat("Camera FOV Angle Y", &state.rendering.cameraFovAngleY, 30.0f, 100.0f)) {
		//	state.rendering.renderingSettings.projectionMatrix = glm::perspective(
		//		glm::radians(state.rendering.cameraFovAngleY),
		//		state.rendering.renderingSettings.aspectRatio(),
		//		state.rendering.renderingSettings.cameraNearDistance,
		//		state.rendering.renderingSettings.cameraFarDistance
		//	);
		//	state.updateFlags.renderingSettingsChanged = true;
		//}
		//// camera movement speed
		//ImGui::SliderFloat("Camera Move Speed", &state.rendering.cameraMoveSpeed, 0.5f, 20.0f);
		//// camera mouse sensitivity
		//ImGui::SliderFloat("Camera Mouse Sensitivity", &state.rendering.cameraMouseSensitivity, 0.01f, 1.0f);

		//if (ImGui::SliderFloat("Camera Near Distance", &state.rendering.renderingSettings.cameraNearDistance, 0.001f, 2.0f)) {
		//	state.rendering.renderingSettings.projectionMatrix = glm::perspective(
		//		glm::radians(state.rendering.cameraFovAngleY),
		//		state.rendering.renderingSettings.aspectRatio(),
		//		state.rendering.renderingSettings.cameraNearDistance,
		//		state.rendering.renderingSettings.cameraFarDistance
		//	);
		//	state.updateFlags.renderingSettingsChanged = true;
		//}
		//if (ImGui::SliderFloat("Camera Far Distance", &state.rendering.renderingSettings.cameraFarDistance, state.rendering.renderingSettings.cameraNearDistance + 0.001f, 200.0f)) {
		//	state.rendering.renderingSettings.projectionMatrix = glm::perspective(
		//		glm::radians(state.rendering.cameraFovAngleY),
		//		state.rendering.renderingSettings.aspectRatio(),
		//		state.rendering.renderingSettings.cameraNearDistance,
		//		state.rendering.renderingSettings.cameraFarDistance
		//	);
		//	state.updateFlags.renderingSettingsChanged = true;
		//}
	}

	ImGui::End();
}


int main() {
	GLFWwindow* window = initializeApplication();

	OpenGLResources glResources;
	DemoState state;

	// initializing GLFW callbacks
	glfwSetWindowUserPointer(window, &state);


	ImGuiIO& io = ImGui::GetIO();

	// main loop
	while (!glfwWindowShouldClose(window)) {
		demos::gui::newFrame();  // TODO - this should probably be elsewhere for the FPS timing to be accurate

		if (state.rendering.cameraMode == CameraMode::FREE_CAMERA) {
			state.translateCamera(
				glfwGetKey(window, CAMERA_MOVE_FORWARDS_KEY) == GLFW_PRESS,
				glfwGetKey(window, CAMERA_MOVE_LEFT_KEY) == GLFW_PRESS,
				glfwGetKey(window, CAMERA_MOVE_BACKWARDS_KEY) == GLFW_PRESS,
				glfwGetKey(window, CAMERA_MOVE_RIGHT_KEY) == GLFW_PRESS,
				glfwGetKey(window, CAMERA_MOVE_UP_KEY) == GLFW_PRESS,
				glfwGetKey(window, CAMERA_MOVE_DOWN_KEY) == GLFW_PRESS,
				io.DeltaTime
			);
		}

		if (state.updateFlags.recompileShaders) {
			glResources.programs = Programs();
			state.updateFlags.recompileShaders = false;
		}

		// update buffer data if demo state changed
		updateBuffers(state, glResources);
		
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// main rendering
		switch (state.rendering.renderingMode) {
			case RenderingMode::ALL_GAUSSIAN_SPLATS:
				drawAllGaussians(state, glResources);				
				break;			
		}

		if (state.rendering.renderAxes) {
			drawAxes(glResources);
		}

		//// read query values
		//// updating light positions
		//if (glResources.queries.directionalLightsPositionUpdatesTimeElapsedQuery.resultAvailable()) {
		//	glResources.queries.directionalLightsPositionUpdatesTimeElapsedQuery.getResult(state.timings.directionalLightsPositionUpdatesMs);
		//}
		//if (glResources.queries.spotLightsPositionUpdatesTimeElapsedQuery.resultAvailable()) {
		//	glResources.queries.spotLightsPositionUpdatesTimeElapsedQuery.getResult(state.timings.spotLightsPositionUpdatesMs);
		//}
		//if (glResources.queries.pointLightsPositionUpdatesTimeElapsedQuery.resultAvailable()) {
		//	glResources.queries.pointLightsPositionUpdatesTimeElapsedQuery.getResult(state.timings.pointLightsPositionUpdatesMs);
		//}
		//// light cluster assignment
		//if (glResources.queries.spotLightClusteringTimeElapsedQuery.resultAvailable()) {
		//	glResources.queries.spotLightClusteringTimeElapsedQuery.getResult(state.timings.spotLightClusteringMs);
		//}
		//if (glResources.queries.pointLightClusteringTimeElapsedQuery.resultAvailable()) {
		//	glResources.queries.pointLightClusteringTimeElapsedQuery.getResult(state.timings.pointLightClusteringMs);
		//}
		//// terrain mesh generation
		//if (glResources.queries.scalarFieldGenerationTimeElapsedQuery.resultAvailable()) {
		//	glResources.queries.scalarFieldGenerationTimeElapsedQuery.getResult(state.timings.scalarFieldGenerationMs);
		//}
		//if (glResources.queries.terrainMeshGenerationTimeElapsedQuery.resultAvailable()) {
		//	glResources.queries.terrainMeshGenerationTimeElapsedQuery.getResult(state.timings.terrainMeshGenerationMs);
		//}
		//// rendering
		//if (glResources.queries.mainRenderingTimeElapsedQuery.resultAvailable()) {
		//	glResources.queries.mainRenderingTimeElapsedQuery.getResult(state.timings.mainRenderingMs);
		//}
		//if (glResources.queries.spotLightsDebugRenderingTimeElapsedQuery.resultAvailable()) {
		//	glResources.queries.spotLightsDebugRenderingTimeElapsedQuery.getResult(state.timings.spotLightsDebugRenderingMs);
		//}
		//if (glResources.queries.pointLightsDebugRenderingTimeElapsedQuery.resultAvailable()) {
		//	glResources.queries.pointLightsDebugRenderingTimeElapsedQuery.getResult(state.timings.pointLightsDebugRenderingMs);
		//}

		if (state.rendering.renderGui && state.rendering.cameraMode != CameraMode::FREE_CAMERA) {
			specifyGuiFrame(state);
		}

		demos::gui::renderFrame();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// free resources
	demos::gui::destroy();

	glfwDestroyWindow(window);  // destroying a window
	glfwTerminate();  // destroys remaining windows and releases resources allocated by GLFW (GLFW has to be reinitialized after this call to work again)

	return EXIT_SUCCESS;
}
