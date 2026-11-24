#include <iostream>
#include <filesystem>

#include "glad/glad.h"
#include "glfw/glfw3.h"
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl3.h"

#include "glwrappers/debug.hpp"

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

	// updating rendering settings and projection matrix
	if (width != 0 && height != 0) {
		state->rendering.renderingSettings.screenWidth = width;
		state->rendering.renderingSettings.screenHeight = height;
		state->rendering.renderingSettings.cameraPrincipalPointInPixels = glm::vec2(static_cast<float>(width) / 2.0f, static_cast<float>(height) / 2.0f);

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
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			glfwSetCursorPos(window, state->rendering.renderingSettings.screenWidth * 0.5, state->rendering.renderingSettings.screenHeight * 0.5);
		}
		else {
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
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
}


void initializeApplication() {
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
}

GLFWwindow* createWindow() {
	// creating a window and setting up context using GLFW
	GLFWwindow* window = glfwCreateWindow(1000, 600, "Single Gaussian Splat Demo", NULL, NULL);
	if (window == NULL) {
		// Window or OpenGL context creation failed
		exit(EXIT_FAILURE);
	}

	// current OpenGL context has to be selected before using OpenGL API
	// remains selected until another one is selected or the window is destroyed
	glfwMakeContextCurrent(window);

	// glfwSwapInterval(0);  // sets the number of screen updates to wait from the time glfwSwapBuffers has been called before swapping buffers

	//gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
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
	// glfwSetScrollCallback(window, scrollCallback);

	// setting up ImGui
	demos::gui::init(window, "#version 460");

	ImGuiIO& io = ImGui::GetIO();

	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // enable keyboard controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;  // enable docking

	// io.IniFilename = nullptr;  // turns off ui persistence

	ImGui::StyleColorsDark();

	//glfwSetWindowMonitor();  // can be used to make window fullscreen
	//glfwSetWindowSize();  // sets window size

	return window;
}


void updateBuffers(DemoState& state, OpenGLResources& glResources) {
	// rendering settings
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

	// gaussian splats array
	if (state.updateFlags.gaussianSplatsChanged) {
		std::span gaussianSplats = state.rendering.gaussianSplats;
		glResources.buffers.gaussianSplatsBuffer.setArrayData(gaussianSplats);
		state.updateFlags.gaussianSplatsChanged = false;
	}

	// (clearing) screen tile counts
	// TODO - each time splats get rendered and counts get updated the flag must be set
	if (state.updateFlags.screenTileSplatCountsChanged) {
		glResources.buffers.screenTileSplatCountsBuffer.fillZero(0, sizeof(GLuint) * state.rendering.renderingSettings.screenTilesPerRow * state.rendering.renderingSettings.screenTilesPerColumn);
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

void orderSplatsInScreenTiles(DemoState& state, OpenGLResources& glResources) {
	//glResources.buffers.screenTileSplatIndicesFirstBuffer.bindShaderStorageBufferToBinding();
	//glResources.buffers.screenTileSplatIndicesSecondBuffer.bindShaderStorageBufferToBinding();

	// querying which buffer is bound to which binding point
	GLuint inputIndexBufferObject = 0;
	GLuint outputIndexBufferObject = 0;
	glGetIntegeri_v(GL_SHADER_STORAGE_BUFFER_BINDING, SCREEN_TILE_SPLAT_INDICES_INPUT_BUFFER_BINDING, (GLint*) &inputIndexBufferObject);
	glGetIntegeri_v(GL_SHADER_STORAGE_BUFFER_BINDING, SCREEN_TILE_SPLAT_INDICES_OUTPUT_BUFFER_BINDING, (GLint*) &outputIndexBufferObject);

	// swapping buffer binding points
	GLuint tmp = inputIndexBufferObject;
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, SCREEN_TILE_SPLAT_INDICES_INPUT_BUFFER_BINDING, outputIndexBufferObject);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, SCREEN_TILE_SPLAT_INDICES_OUTPUT_BUFFER_BINDING, tmp);
	inputBufferObj = tmp = outputBufferObj;
	outputBufferObj = tmp;

	for (size_t globalComparisonBlockSize = INITIAL_GLOBAL_COMPARISON_BLOCK_SIZE; globalComparisonBlockSize <= arrayLength; globalComparisonBlockSize <<= 1ull) {
		if (globalComparisonBlockSize > INITIAL_GLOBAL_COMPARISON_BLOCK_SIZE) {
			// swap inputArray and outputArray after finalSharedMemorySwaps kernel left what is to be the new input in outputArray
			
			int* temp = inputArray;
			inputArray = outputArray;
			outputArray = temp;
		}

		size_t comparisonBlockSize = globalComparisonBlockSize;
		// sorting the current global blocks in global memory until the swaps can fit into shared memory
		while (comparisonBlockSize > maxSharedMemoryValues) {
			intermediateGlobalMemorySwap << <threadGridSize, threadBlockSize, 0, stream >> > (inputArray, outputArray, arrayLength, globalComparisonBlockSize, comparisonBlockSize, ascending);
			glResources.programs.orderSplatsInScreenTilesGlobalMemoryProgram.use();

			glResources.buffers.screenTileSortingParametersBuffer.setData(state.);

			glDispatchCompute();

			// // check for any errors when launching the kernel
			// // cudaDeviceSynchronize waits for the kernel to finish
			// cudaStatus = cudaDeviceSynchronize();

			// swap inputArray and outputArray after each step

			int* temp = inputArray;
			inputArray = outputArray;
			outputArray = temp;

			comparisonBlockSize >>= 1ull;
		}

		finalSharedMemorySwaps << <threadGridSize, threadBlockSize, sharedMemorySizeBytes, stream >> > (inputArray, outputArray, arrayLength, globalComparisonBlockSize, comparisonBlockSize, ascending);

		glResources.programs.orderSplatsInScreenTilesGlobalMemoryProgram.use();
		glDispatchCompute();

		// // check for any errors when launching the kernel
		// // cudaDeviceSynchronize waits for the kernel to finish
		// cudaStatus = cudaDeviceSynchronize();

		// synchronizing stream
		// check for any errors when launching the kernels
	}

}

void drawAllGaussians(DemoState& state, OpenGLResources& glResources) {
	// assign gaussian to tiles
	glResources.programs.assignSplatsToScreenTilesProgram.use();
	glDispatchCompute((state.rendering.renderingSettings.totalGaussians + 255) / 256, 1, 1);

	// order gaussians in tiles
	orderSplatsInScreenTiles(state, glResources);

	// render ordered gaussians
	glResources.programs.renderTiledSplatsToTextureProgram.use();

	//glResources.queries.mainRenderingTimeElapsedQuery.begin();
	glDispatchCompute(
		(__min(glResources.textures.textureWidth, state.rendering.renderingSettings.screenWidth) + 15) / 16,
		(__min(glResources.textures.textureHeight, state.rendering.renderingSettings.screenHeight) + 15) / 16,
		1
	);
	//glResources.queries.mainRenderingTimeElapsedQuery.end();
	glResources.textures.blitToScreen(state.rendering.renderingSettings.screenWidth, state.rendering.renderingSettings.screenHeight);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

