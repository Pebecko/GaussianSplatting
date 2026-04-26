#include <iostream>
#include <filesystem>

#include <glad/glad.h>
#include <glfw/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>

#include "glWrappers/debug.hpp"
#include "demoFramework/shaderCodeManipulation.hpp"
#include "demoFramework/imguiWrapper.hpp"
#include "demoFramework/resourcePool.hpp"

//#include "glfwCallbacks.hpp"
#include "openglResources.hpp"
#include "demoState.hpp"


// resources:
// http://www.aortiz.me/2018/12/21/CG.html#part-2
// https://www.cse.chalmers.se/~uffe/clustered_shading_preprint.pdf
// https://www.activision.com/cdn/research/2017_Sig_Improved_Culling_final.pdf
// https://www.humus.name/Articles/PracticalClusteredShading.pdf


// polishing roadmap:
//	* Free camera
//		- update position each frame
//	* More lights
//		- turn the orbit UBOs into SSBOs
//	* Build spot light clusters!!!
//	* Multipliers for scaling values
//		- allow changing how many lights of each type are taken into account
//		- change the sizes of light clusters
//		- set type of cluster z slicing (linear, logarithmic)
//		- scale material and light coefficients
//		- scale light speed
//		- change camera properties
//		- independent scaling of point radii and light radii
//		- find some better default constant values and constant ranges
//		- change how the number of generated chunks is specified (1D -> 3D)
//	* Combine some settings UBOs
//	* Model!!!
//		- add model loading
//		- add model mesh rendering
//	* Start measuring timings
//	* Cluster debug rendering (how?!?!?!)
//		- could somehow render a grid of z slices with each x/y cluster being assigned a color based on number of lights
//	* update chunksDispatch whenever chunksPerAxis changes
//	* Add and option to "PAUSE TIME" (stop updating light positions for easier verification)
//	* (figure out how to draw spot lights differently than point lights!)
//	* compute light radius based on light parameters!!!
//	* check that normals are being transformed correctly in mesh rendering vertex shaders


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

	if (width != 0 && height != 0) {
		state->rendering.renderingSettings.screenWidth = width;
		state->rendering.renderingSettings.screenHeight = height;

		state->rendering.renderingSettings.projectionMatrix = glm::perspective(
			glm::radians(state->rendering.cameraFovAngleY),
			state->rendering.renderingSettings.aspectRatio(),
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


void specifyGuiFrame(DemoState& state) {
	ImGui::Begin("Single Splat Demo - Settings");

	ImGuiIO& io = ImGui::GetIO();

	if (ImGui::Button("Recompile Shaders")) {
		state.updateFlags.recompileShaders = true;
	}

	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f * io.DeltaTime, io.Framerate);
	// could later have some submenu for keybinds
	if (ImGui::CollapsingHeader("Keybinds", ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::Text("F1 - switch between camera modes");
		ImGui::Text("F2 - toggle GUI visibility");
		ImGui::Text("W, A, S, D, Space, LShift - camera control");
	}

	// TODO - application timing submenu
	if (ImGui::CollapsingHeader("Time Measurements", ImGuiTreeNodeFlags_DefaultOpen)) {
		// data buffering timing
		// could use it both for general buffer updates and specifically when comparing CPU generation + data buffering

		// rendering timing
		ImGui::Text(
			"Total RENDERING time: %lu ns",
			state.timings.mainRenderingMs + state.timings.spotLightsDebugRenderingMs + state.timings.pointLightsDebugRenderingMs
		);
		ImGui::Text(
			"  Main rendering: %lu ns, Spot lights debug: %lu ns, Point lights debug: %lu ns",
			state.timings.mainRenderingMs,
			state.timings.spotLightsDebugRenderingMs,
			state.timings.pointLightsDebugRenderingMs
		);
	}

	// terrain generation settings submenu
	static const char* backendNames[] = {
		//"CPU",					     // GENERATION_BACKEND_CPU
		//"GPU (Single Invocation)", 	 // GENERATION_BACKEND_GPU_SINGLE_INVOCATION
		"GPU (Single Workgroup)",  	 // GENERATION_BACKEND_GPU_SINGLE_WORKGROUP
		//"GPU (Multiple Workgroups)"	 // GENERATION_BACKEND_GPU_MULTIPLE_WORKGROUPS
	};

	if (ImGui::CollapsingHeader("Terrain Mesh Generation Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
		// dropdown for selecting generation backend
		if (ImGui::Combo("Chunk Mesh Generation Backend", &backend, backendNames, IM_ARRAYSIZE(backendNames))) {
			state.generation.generationBackend = GenerationBackend(backend);
		}

		// uint slider for each dimension of chunk grid
		if (ImGui::SliderInt3("Chunks Per Axis", chunksPerAxis, 1, __min(__min(MAX_CHUNKS_X, MAX_CHUNKS_Y), MAX_CHUNKS_Z))) {  // the bound might be problematic
			state.generation.chunksPerAxis = glm::uvec3(chunksPerAxis[0], chunksPerAxis[1], chunksPerAxis[2]);
		}

		// float slider for each dimension of chunkGridOrigin
		if (ImGui::SliderFloat3("Chunk Grid Origin", glm::value_ptr(state.generation.generationSettings.chunkGridOrigin), -64.0f, 64.0f)) {
			state.updateFlags.generationSettingsChanged = true;
		}
		// uint slider for each dimension of chunk size
		if (ImGui::SliderInt3("Chunk Nodes Per Axis", chunkNodesPerAxis, MIN_CHUNK_DIMENSION_SIZE, MAX_CHUNK_DIMENSION_SIZE)) {
			state.generation.generationSettings.chunkNodesPerAxis = glm::uvec3(chunkNodesPerAxis[0], chunkNodesPerAxis[1], chunkNodesPerAxis[2]);
			state.updateFlags.generationSettingsChanged = true;
		}
		// float slider for each dimension of chunkNodeSpacing
		if (ImGui::SliderFloat3("Chunk Grid Node Spacing", glm::value_ptr(state.generation.generationSettings.chunkNodeSpacing), 0.01f, 2.0f)) {
			state.updateFlags.generationSettingsChanged = true;
		}

		// float slider for each dimension of noise grid origin
		if (ImGui::SliderFloat3("Noise Grid Origin", glm::value_ptr(state.generation.generationSettings.noiseGridOrigin), -256.0f, 256.0f)) {
			state.updateFlags.generationSettingsChanged = true;
		}
		// float slider for each dimension of noise grid base edge length
		if (ImGui::SliderFloat3("Noise Grid Node Spacing", glm::value_ptr(state.generation.generationSettings.noiseGridNodeSpacing), 0.01f, 128.0f)) {
			state.updateFlags.generationSettingsChanged = true;
		}
		// noise seed input
		if (ImGui::InputInt("Noise Seed", &state.generation.generationSettings.globalNoiseSeed)) {
			state.updateFlags.generationSettingsChanged = true;
		}
		// uint slider for totalNoiseOctaves
		if (ImGui::SliderInt("Total Noise Octaves", &octaves, 1, 12)) {
			state.generation.generationSettings.totalNoiseOctaves = uint32_t(octaves);
			state.updateFlags.generationSettingsChanged = true;
		}
		// noise frequency multiplier
		if (ImGui::SliderFloat("Noise Frequency Multiplier", &state.generation.generationSettings.noiseFrequencyMultiplier, 0.01f, 4.0f)) {
			state.updateFlags.generationSettingsChanged = true;
		}
		// noise amplitude multiplier
		if (ImGui::SliderFloat("Noise Amplitude Multiplier", &state.generation.generationSettings.noiseAmplitudeMultiplier, 0.01f, 4.0f)) {
			state.updateFlags.generationSettingsChanged = true;
		}

		// isolevel
		if (ImGui::SliderFloat("Isolevel", &state.generation.generationSettings.isolevel, -1.0f, 1.0f)) {
			state.updateFlags.generationSettingsChanged = true;
		}
	}

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
		// dropdown for selecting rendering mode
		if (ImGui::Combo("Rendering Mode", &renderingMode, renderingModeNames, IM_ARRAYSIZE(renderingModeNames))) {
			state.rendering.renderingMode = RenderingMode(renderingMode);
		}

		// checkbox for uniform point size
		if (ImGui::Checkbox("Uniform Point Size", &state.rendering.uniformPointSize)) {
			state.updateFlags.renderingSettingsChanged = true;
		}
		// slider for basePointRadius
		if (ImGui::SliderFloat("Base Point Radius", &state.rendering.renderingSettings.basePointRadius, 1.0f, 256.0f)) {
			state.updateFlags.renderingSettingsChanged = true;
		}
		// slider for baseLightRadius
		if (ImGui::SliderFloat("Base Light Radius", &state.rendering.renderingSettings.baseLightRadius, 0.1f, 128.0f)) {
			state.updateFlags.renderingSettingsChanged = true;
		}

		// TODO - consider adding camera mode

		// camera Y FOV angle
		if (ImGui::SliderFloat("Camera FOV Angle Y", &state.rendering.cameraFovAngleY, 30.0f, 100.0f)) {
			state.rendering.renderingSettings.projectionMatrix = glm::perspective(
				glm::radians(state.rendering.cameraFovAngleY),
				state.rendering.renderingSettings.aspectRatio(),
				state.rendering.renderingSettings.cameraNearDistance,
				state.rendering.renderingSettings.cameraFarDistance
			);
			state.updateFlags.renderingSettingsChanged = true;
		}

		// camera movement speed
		ImGui::SliderFloat("Camera Move Speed", &state.rendering.cameraMoveSpeed, 0.5f, 20.0f);
		// camera mouse sensitivity
		ImGui::SliderFloat("Camera Mouse Sensitivity", &state.rendering.cameraMouseSensitivity, 0.01f, 1.0f);

		// light debug rendering checkboxes
		ImGui::Checkbox("Render Debug Spot Lights", &state.rendering.renderSpotLightsDebugPoints);
		ImGui::Checkbox("Render Debug Point Lights", &state.rendering.renderPointLightsDebugPoints);

		if (ImGui::SliderFloat("Camera Near Distance", &state.rendering.renderingSettings.cameraNearDistance, 0.001f, 2.0f)) {
			state.rendering.renderingSettings.projectionMatrix = glm::perspective(
				glm::radians(state.rendering.cameraFovAngleY),
				state.rendering.renderingSettings.aspectRatio(),
				state.rendering.renderingSettings.cameraNearDistance,
				state.rendering.renderingSettings.cameraFarDistance
			);
			state.updateFlags.renderingSettingsChanged = true;
		}
		if (ImGui::SliderFloat("Camera Far Distance", &state.rendering.renderingSettings.cameraFarDistance, state.rendering.renderingSettings.cameraNearDistance + 0.001f, 200.0f)) {
			state.rendering.renderingSettings.projectionMatrix = glm::perspective(
				glm::radians(state.rendering.cameraFovAngleY),
				state.rendering.renderingSettings.aspectRatio(),
				state.rendering.renderingSettings.cameraNearDistance,
				state.rendering.renderingSettings.cameraFarDistance
			);
			state.updateFlags.renderingSettingsChanged = true;
		}
	}

	// lighting settings submenu
	int usedDirectionalLights = int(state.lighting.lightingParameters.usedDirectionalLights);
	int usedSpotLights = int(state.lighting.lightingParameters.usedSpotLights);
	int usedPointLights = int(state.lighting.lightingParameters.usedPointLights);

	static const char* clusterZSpacingModeNames[] = {
		"Linear Z Layer Spacing",  // LINEAR
		"Logarithmic Z Layer Spacing",  // LOGARITHMIC
	};
	int clusterZSpacingMode = int(state.lighting.lightingParameters.clusterZLayerSpacingMode);

	// this doesn't work
	glm::uvec3 minLightClusters(MIN_LIGHT_CLUSTERS_X, MIN_LIGHT_CLUSTERS_Y, MIN_LIGHT_CLUSTERS_Z);
	glm::uvec3 maxLightClusters(MAX_LIGHT_CLUSTERS_X, MAX_LIGHT_CLUSTERS_Y, MAX_LIGHT_CLUSTERS_Z);

	glm::vec3 directionalLightCoefficients = glm::vec3(
		state.lighting.lightingParameters.directionalLightAmbientCoefficient,
		state.lighting.lightingParameters.directionalLightDiffuseCoefficient,
		state.lighting.lightingParameters.directionalLightSpecularCoefficient
	);
	glm::vec3 spotLightCoefficients = glm::vec3(
		state.lighting.lightingParameters.spotLightAmbientCoefficient,
		state.lighting.lightingParameters.spotLightDiffuseCoefficient,
		state.lighting.lightingParameters.spotLightSpecularCoefficient
	);
	glm::vec3 pointLightCoefficients = glm::vec3(
		state.lighting.lightingParameters.pointLightAmbientCoefficient,
		state.lighting.lightingParameters.pointLightDiffuseCoefficient,
		state.lighting.lightingParameters.pointLightSpecularCoefficient
	);

	glm::vec4 materialCoefficients = glm::vec4(
		state.lighting.lightingParameters.materialAmbientCoefficient,
		state.lighting.lightingParameters.materialDiffuseCoefficient,
		state.lighting.lightingParameters.materialSpecularCoefficient,
		state.lighting.lightingParameters.materialEmissiveCoefficient
	);

	if (ImGui::CollapsingHeader("Lighting Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
		// checkbox for lighting time update (movement of lights)
		if (ImGui::Checkbox("Update Light Time (Positions)", &state.lighting.updateCurrentTime)) {
		}

// uint slider for usedDirectionalLights
		if (ImGui::SliderInt("Total Directional Lights", &usedDirectionalLights, 0, MAX_DIRECTIONAL_LIGHTS)) {
			state.lighting.lightingParameters.usedDirectionalLights = GLuint(usedDirectionalLights);
			state.updateFlags.lightingParametersChanged = true;
		}
		// uint slider for usedSpotLights
		if (ImGui::SliderInt("Total Spot Lights", &usedSpotLights, 0, MAX_SPOT_LIGHTS)) {
			state.lighting.lightingParameters.usedSpotLights = GLuint(usedSpotLights);
			state.updateFlags.lightingParametersChanged = true;
		}
		// uint slider for usedPointLights
		if (ImGui::SliderInt("Total Point Lights", &usedPointLights, 0, MAX_POINT_LIGHTS)) {
			state.lighting.lightingParameters.usedPointLights = GLuint(usedPointLights);
			state.updateFlags.lightingParametersChanged = true;
		}

		// the bounds don't work as intended
		if (ImGui::SliderScalarN("Light Clusters Per Axis", ImGuiDataType_U32, glm::value_ptr(state.lighting.lightingParameters.clustersPerAxis), 3, glm::value_ptr(minLightClusters), glm::value_ptr(maxLightClusters))) {
			state.updateFlags.lightingParametersChanged = true;
		}

		// using "%.2e" as format for scientific notations doesn't work well
		if (ImGui::SliderFloat("Min Clustering Light Atteuation Threshold", &state.lighting.lightingParameters.minClusteredAttenuationThreshold, 0.000001f, 0.99f, "%.6f", ImGuiSliderFlags_Logarithmic)) {
			state.updateFlags.lightingParametersChanged = true;
		}

		if (ImGui::Combo("Light Clusters Z Layer Spacing Mode", &clusterZSpacingMode, clusterZSpacingModeNames, IM_ARRAYSIZE(clusterZSpacingModeNames))) {
			state.lighting.lightingParameters.clusterZLayerSpacingMode = LightClusterZLayerSpacingMode(clusterZSpacingMode);
		}

		if (ImGui::SliderFloat3("Directional Lights Coeffs: AMBIENT | DIFFUSE | SPECULAR", glm::value_ptr(directionalLightCoefficients), 0.001f, 1.0f)) {
			state.lighting.lightingParameters.directionalLightAmbientCoefficient = directionalLightCoefficients.x;
			state.lighting.lightingParameters.directionalLightDiffuseCoefficient = directionalLightCoefficients.y;
			state.lighting.lightingParameters.directionalLightSpecularCoefficient = directionalLightCoefficients.z;
			state.updateFlags.lightingParametersChanged = true;
		}
		if (ImGui::SliderFloat3("Spot Lights Coeffs: AMBIENT | DIFFUSE | SPECULAR", glm::value_ptr(spotLightCoefficients), 0.001f, 1.0f)) {
			state.lighting.lightingParameters.spotLightAmbientCoefficient = spotLightCoefficients.x;
			state.lighting.lightingParameters.spotLightDiffuseCoefficient = spotLightCoefficients.y;
			state.lighting.lightingParameters.spotLightSpecularCoefficient = spotLightCoefficients.z;
			state.updateFlags.lightingParametersChanged = true;
		}
		if (ImGui::SliderFloat3("Point Lights Coeffs: AMBIENT | DIFFUSE | SPECULAR", glm::value_ptr(pointLightCoefficients), 0.001f, 1.0f)) {
			state.lighting.lightingParameters.pointLightAmbientCoefficient = pointLightCoefficients.x;
			state.lighting.lightingParameters.pointLightDiffuseCoefficient = pointLightCoefficients.y;
			state.lighting.lightingParameters.pointLightSpecularCoefficient = pointLightCoefficients.z;
			state.updateFlags.lightingParametersChanged = true;
		}

		if (ImGui::SliderFloat4("Material Coeffs: AMBIENT | DIFFUSE | SPECULAR | EMMISIVE", glm::value_ptr(materialCoefficients), 0.001f, 1.0f)) {
			state.lighting.lightingParameters.materialAmbientCoefficient = materialCoefficients.x;
			state.lighting.lightingParameters.materialDiffuseCoefficient = materialCoefficients.y;
			state.lighting.lightingParameters.materialSpecularCoefficient = materialCoefficients.z;
			state.lighting.lightingParameters.materialEmissiveCoefficient = materialCoefficients.w;
			state.updateFlags.lightingParametersChanged = true;
		}

		// float slider for global ambient light
		if (ImGui::ColorEdit3("Global Ambient Light", glm::value_ptr(state.lighting.lightingParameters.globalAmbientLight))) {
			state.updateFlags.lightingParametersChanged = true;
		}

		// orbits center offset
		if (ImGui::SliderFloat3("Orbits Center Offset", glm::value_ptr(state.lighting.lightingParameters.orbitsCenterOffset), -50.0f, 50.0f)) {
			state.updateFlags.lightingParametersChanged = true;
		}
		// orbits angular velocity coefficient
		if (ImGui::SliderFloat("Orbits Angular Velocity Offset", &state.lighting.lightingParameters.orbitAngularVelocityCoefficient, 0.05f, 5.0f)) {
			state.updateFlags.lightingParametersChanged = true;
		}
	}

	ImGui::End();
}


int main() {
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
	GLFWwindow* window = glfwCreateWindow(1000, 600, "Clustered forward shading demo", NULL, NULL);
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
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	// depth test (https://wikis.khronos.org/opengl/Depth_Test)
	glDepthFunc(GL_LESS);
	glEnable(GL_DEPTH_TEST);

	glClearColor(0.9f, 0.6f, 0.9f, 1.0f);

	// initializing OpenGL resources
	OpenGLResources glResources;

	// initializing demo state
	DemoState state;

	// initializing GLFW callbacks
	glfwSetWindowUserPointer(window, &state);

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

		if (state.lighting.updateCurrentTime) {
			state.lighting.lightingParameters.currentTime = (float) ImGui::GetTime();  // TODO - use chrono or something instead
			state.updateFlags.lightingParametersChanged = true;
		}

		if (state.updateFlags.chunkGridDimensionsChanged) {
			for (unsigned int chunkIndexZ = 0u; chunkIndexZ < state.generation.chunksPerAxis.z; ++chunkIndexZ) {
				for (unsigned int chunkIndexY = 0u; chunkIndexY < state.generation.chunksPerAxis.y; ++chunkIndexY) {
					for (unsigned int chunkIndexX = 0u; chunkIndexX < state.generation.chunksPerAxis.x; ++chunkIndexX) {
						unsigned int linearChunkIndex = state.generation.chunksPerAxis.x * state.generation.chunksPerAxis.y * chunkIndexZ + state.generation.chunksPerAxis.x * chunkIndexY + chunkIndexX;

						if (linearChunkIndex < state.generation.chunksDispatch.size()) {  // modifying existing values
							state.generation.chunksDispatch[linearChunkIndex].chunkGridPosition = glm::ivec3(chunkIndexX, chunkIndexY, chunkIndexZ);
						}
						else {
							state.generation.chunksDispatch.push_back(
								ChunkDispatch(
									glm::ivec3(chunkIndexX, chunkIndexY, chunkIndexZ),
									linearChunkIndex * MAX_CHUNK_MEMORY_ELEMENTS,
									linearChunkIndex,
									linearChunkIndex * MAX_CHUNK_ELEMENTS * MAX_VERTICES_FROM_CUBE,
									linearChunkIndex * MAX_CHUNK_ELEMENTS * MAX_NORMALS_FROM_CUBE
								)
							);
						}
					}
				}
			}

			state.updateFlags.chunksDispatchChanged = true;
		}

		// update buffer data if demo state changed
		if (state.updateFlags.generationSettingsChanged) {
			glResources.buffers.generationSettingsBuffer.setData(state.generation.generationSettings);
			state.updateFlags.generationSettingsChanged = false;
			state.updateFlags.scalarFieldGenerationNeeded = true;
		}
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
		if (state.updateFlags.lightingParametersChanged) {
			glResources.buffers.lightingParametersBuffer.setData(state.lighting.lightingParameters);
			state.updateFlags.lightingParametersChanged = false;
		}

		if (state.updateFlags.chunksDispatchChanged) {  // chunk dispatch needs to be updated when chunksPerAxisChange
			glResources.buffers.chunksDispatchBuffer.setArrayData(std::span(state.generation.chunksDispatch.data(), state.generation.chunksDispatch.size()));
			state.updateFlags.chunksDispatchChanged = false;
			state.updateFlags.scalarFieldGenerationNeeded = true;
		}
		if (state.updateFlags.drawCommandsChanged) {
			state.updateFlags.drawCommandsChanged = false;
			state.updateFlags.scalarFieldGenerationNeeded = true;
		}

		if (state.updateFlags.directionalLightsChanged) {
			std::span directionalLights = state.lighting.directionalLights;
			glResources.buffers.directionalLightsBuffer.setArrayData(directionalLights);
			state.updateFlags.directionalLightsChanged = false;
		}
		if (state.updateFlags.pointLightsChanged) {
			std::span pointLights = state.lighting.pointLights;
			glResources.buffers.pointLightsBuffer.setArrayData(pointLights);
			state.updateFlags.pointLightsChanged = false;
		}
		if (state.updateFlags.spotLightsChanged) {
			std::span spotLights = state.lighting.spotLights;
			glResources.buffers.spotLightsBuffer.setArrayData(spotLights);
			state.updateFlags.spotLightsChanged = false;
		}
		if (state.updateFlags.materialsChanged) {
			std::span materials = state.lighting.materials;
			glResources.buffers.materialsBuffer.setArrayData(materials);
			state.updateFlags.materialsChanged = false;
		}

		if (state.updateFlags.directionalLightOrbitsChanged) {
			std::span directionalLightOrbits = state.lighting.directionalLightOrbits;
			glResources.buffers.directionalLightOrbits.setArrayData(directionalLightOrbits);
			state.updateFlags.directionalLightOrbitsChanged = false;
		}
		if (state.updateFlags.spotLightOrbitsChanged) {
			std::span spotLightOrbits = state.lighting.spotLightOrbits;
			glResources.buffers.spotLightOrbits.setArrayData(spotLightOrbits);
			state.updateFlags.spotLightOrbitsChanged = false;
		}
		if (state.updateFlags.pointLightOrbitsChanged) {
			std::span pointLightOrbits = state.lighting.pointLightOrbits;
			glResources.buffers.pointLightOrbits.setArrayData(pointLightOrbits);
			state.updateFlags.pointLightOrbitsChanged = false;
		}

		if (state.updateFlags.clusterSpotLightIndicesOffsetsChanged) {
			std::span clusterSpotLightIndicesOffsets = state.lighting.clusterSpotLightIndicesOffsets;
			glResources.buffers.clusterSpotLightIndicesOffsets.setArrayData(clusterSpotLightIndicesOffsets);
			state.updateFlags.clusterSpotLightIndicesOffsetsChanged = false;
		}
		if (state.updateFlags.clusterPointLightIndicesOffsetsChanged) {
			std::span clusterPointLightIndicesOffsets = state.lighting.clusterPointLightIndicesOffsets;
			glResources.buffers.clusterPointLightIndicesOffsets.setArrayData(clusterPointLightIndicesOffsets);
			state.updateFlags.clusterPointLightIndicesOffsetsChanged = false;
		}
		// TODO - add updates to the other buffers when needed

		glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);

		// updating light positions
		glResources.programs.updateDirectionalLightPositionProgram.use();
		glResources.queries.directionalLightsPositionUpdatesTimeElapsedQuery.begin();
		glDispatchCompute(state.lighting.lightingParameters.usedDirectionalLights, 1u, 1u);
		glResources.queries.directionalLightsPositionUpdatesTimeElapsedQuery.end();

		glResources.programs.updateSpotLightPositionProgram.use();
		glResources.queries.spotLightsPositionUpdatesTimeElapsedQuery.begin();
		glDispatchCompute(state.lighting.lightingParameters.usedSpotLights, 1u, 1u);
		glResources.queries.spotLightsPositionUpdatesTimeElapsedQuery.end();

		glResources.programs.updatePointLightPositionProgram.use();
		glResources.queries.pointLightsPositionUpdatesTimeElapsedQuery.begin();
		glDispatchCompute(state.lighting.lightingParameters.usedPointLights, 1u, 1u);
		glResources.queries.pointLightsPositionUpdatesTimeElapsedQuery.end();

		glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);

		// generating scalar field
		if (state.updateFlags.scalarFieldGenerationNeeded) {
			glResources.programs.scalarFieldGenerationProgram.use();

			glResources.queries.scalarFieldGenerationTimeElapsedQuery.begin();
			glDispatchCompute(state.generation.totalChunks(), 1u, 1u);
			glResources.queries.scalarFieldGenerationTimeElapsedQuery.end();

			glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);  // scalar field has to finish generating before mesh can be generated from it
			state.updateFlags.scalarFieldGenerationNeeded = false;
			state.updateFlags.terrainMeshGenerationNeeded = true;
		}

		// could build the clusters only when needed for rendering
		// assigning lights to clusters
		// clearing the light counts arrays
		glResources.buffers.clusterSpotLightCounts.fillZero();
		glResources.buffers.clusterPointLightCounts.fillZero();
		glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);

		// spot lights
		glResources.programs.findSpotLightClustersProgram.use();
		glResources.queries.spotLightClusteringTimeElapsedQuery.begin();
		glDispatchCompute(state.lighting.lightingParameters.usedSpotLights, 1u, 1u);
		glResources.queries.spotLightClusteringTimeElapsedQuery.end();

		// point lights
		glResources.programs.findPointLightClustersProgram.use();
		glResources.queries.pointLightClusteringTimeElapsedQuery.begin();
		glDispatchCompute(state.lighting.lightingParameters.usedPointLights, 1u, 1u);
		glResources.queries.pointLightClusteringTimeElapsedQuery.end();

		// generating chunk mesh
		if (state.updateFlags.terrainMeshGenerationNeeded) {
			glResources.programs.meshGenerationProgram_oneWorkGroup_atomic.use();
			glResources.queries.terrainMeshGenerationTimeElapsedQuery.begin();
			glDispatchCompute(state.generation.totalChunks(), 1u, 1u);
			glResources.queries.terrainMeshGenerationTimeElapsedQuery.end();
			glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);  // mesh has to finish generating before it can be rendered
			state.updateFlags.terrainMeshGenerationNeeded = false;
		};

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// rendering the scalar field/chunk mesh based on the demo settings
		//GLsizei totalChunkCubesNodes = state.generationSettings.chunkNodesPerAxis.x * state.generationSettings.chunkNodesPerAxis.y * state.generationSettings.chunkNodesPerAxis.z;

		switch (state.rendering.renderingMode) {
			case RenderingMode::SCALAR_FIELD:
				glResources.buffers.emptyVAO.bind();

				glResources.programs.scalarFieldRenderingProgram.use();
				glResources.queries.mainRenderingTimeElapsedQuery.begin();
				glDrawArraysInstanced(GL_POINTS, /* gl_VertexID offset */ 0, state.generation.totalChunkMemoryNodes(), state.generation.totalChunks());
				glResources.queries.mainRenderingTimeElapsedQuery.end();

				break;
			case RenderingMode::CHUNK_MESH_NAIVE:
				glResources.buffers.emptyVAO.bind();
				glResources.buffers.indirectDrawCommandBuffer.bind(GL_DRAW_INDIRECT_BUFFER);

				glResources.programs.terrainMeshRenderingProgram_naive.use();
				glResources.queries.mainRenderingTimeElapsedQuery.begin();
				glMultiDrawArraysIndirect(GL_TRIANGLES, /* byte offset into the bound L_DRAW_INDIRECT_BUFFER */ (void*) 0, /* total draws */ state.generation.totalChunks(), /* indirect buffer commands stride */ 0);
				glResources.queries.mainRenderingTimeElapsedQuery.end();

				break;
			case RenderingMode::CHUCK_MESH_CLUSTERED:
				glResources.buffers.emptyVAO.bind();
				glResources.buffers.indirectDrawCommandBuffer.bind(GL_DRAW_INDIRECT_BUFFER);

				glResources.programs.terrainMeshRenderingProgram_clustered.use();
				glResources.queries.mainRenderingTimeElapsedQuery.begin();
				glMultiDrawArraysIndirect(GL_TRIANGLES, /* byte offset into the bound L_DRAW_INDIRECT_BUFFER */ (void*) 0, /* total draws */ state.generation.totalChunks(), /* indirect buffer commands stride */ 0);
				glResources.queries.mainRenderingTimeElapsedQuery.end();

				break;
			case RenderingMode::MODEL_MESH_NAIVE:
				glResources.buffers.modelVAO.bind();
				glResources.programs.modelMeshRenderingProgram_naive.use();
				glResources.queries.mainRenderingTimeElapsedQuery.begin();
				//glMultiDrawArraysIndirect(GL_TRIANGLES, /* byte offset into the bound L_DRAW_INDIRECT_BUFFER */ (void*) 0, /* total draws */ state.generation.totalChunks(), /* indirect buffer commands stride */ 0);
				glDrawArraysInstanced(GL_TRIANGLES, 0, /* model vertices */ 3, 1);
				glResources.queries.mainRenderingTimeElapsedQuery.end();

				break;
			case RenderingMode::MODEL_MESH_CLUSTERED:
				glResources.buffers.modelVAO.bind();
				glResources.programs.modelMeshRenderingProgram_clustered.use();
				glResources.queries.mainRenderingTimeElapsedQuery.begin();
				//glMultiDrawArraysIndirect(GL_TRIANGLES, /* byte offset into the bound L_DRAW_INDIRECT_BUFFER */ (void*) 0, /* total draws */ state.generation.totalChunks(), /* indirect buffer commands stride */ 0);
				glDrawArraysInstanced(GL_TRIANGLES, 0, /* model vertices */ 3, 1);
				glResources.queries.mainRenderingTimeElapsedQuery.end();

				break;
			case RenderingMode::CHUNK_CLUSTER_INDICES_DEBUG:
				glResources.buffers.emptyVAO.bind();
				glResources.buffers.indirectDrawCommandBuffer.bind(GL_DRAW_INDIRECT_BUFFER);

				glResources.programs.terrainClusterIndicesDebugRenderingProgram.use();

				glResources.queries.mainRenderingTimeElapsedQuery.begin();
				glMultiDrawArraysIndirect(GL_TRIANGLES, /* byte offset into the bound L_DRAW_INDIRECT_BUFFER */ (void*) 0, /* total draws */ state.generation.totalChunks(), /* indirect buffer commands stride */ 0);
				glResources.queries.mainRenderingTimeElapsedQuery.end();

				break;
			case RenderingMode::CHUNK_CLUSTER_OCCUPANCY_DEBUG:
				glResources.buffers.emptyVAO.bind();
				glResources.buffers.indirectDrawCommandBuffer.bind(GL_DRAW_INDIRECT_BUFFER);

				glResources.programs.terrainClusterOccupancyDebugRenderingProgram.use();

				glResources.queries.mainRenderingTimeElapsedQuery.begin();
				glMultiDrawArraysIndirect(GL_TRIANGLES, /* byte offset into the bound L_DRAW_INDIRECT_BUFFER */ (void*) 0, /* total draws */ state.generation.totalChunks(), /* indirect buffer commands stride */ 0);
				glResources.queries.mainRenderingTimeElapsedQuery.end();

				break;
		}

		// rendering of these should probably also be timed somehow
		if (state.rendering.renderSpotLightsDebugPoints) {
			glResources.programs.spotLightDebugRenderingProgram.use();

			glResources.queries.spotLightsDebugRenderingTimeElapsedQuery.begin();
			glDrawArrays(GL_POINTS, 0, state.lighting.lightingParameters.usedSpotLights);
			glResources.queries.spotLightsDebugRenderingTimeElapsedQuery.end();
		}

		// rendering of these should probably also be timed somehow
		if (state.rendering.renderPointLightsDebugPoints) {
			glResources.programs.pointLightDebugRenderingProgram.use();

			glResources.queries.pointLightsDebugRenderingTimeElapsedQuery.begin();
			glDrawArrays(GL_POINTS, 0, state.lighting.lightingParameters.usedPointLights);
			glResources.queries.pointLightsDebugRenderingTimeElapsedQuery.end();
		}

		glFinish();

		// read query values
		// updating light positions
		if (glResources.queries.directionalLightsPositionUpdatesTimeElapsedQuery.resultAvailable()) {
			glResources.queries.directionalLightsPositionUpdatesTimeElapsedQuery.getResult(state.timings.directionalLightsPositionUpdatesMs);
		}
		if (glResources.queries.spotLightsPositionUpdatesTimeElapsedQuery.resultAvailable()) {
			glResources.queries.spotLightsPositionUpdatesTimeElapsedQuery.getResult(state.timings.spotLightsPositionUpdatesMs);
		}
		if (glResources.queries.pointLightsPositionUpdatesTimeElapsedQuery.resultAvailable()) {
			glResources.queries.pointLightsPositionUpdatesTimeElapsedQuery.getResult(state.timings.pointLightsPositionUpdatesMs);
		}

		// light cluster assignment
		if (glResources.queries.spotLightClusteringTimeElapsedQuery.resultAvailable()) {
			glResources.queries.spotLightClusteringTimeElapsedQuery.getResult(state.timings.spotLightClusteringMs);
		}
		if (glResources.queries.pointLightClusteringTimeElapsedQuery.resultAvailable()) {
			glResources.queries.pointLightClusteringTimeElapsedQuery.getResult(state.timings.pointLightClusteringMs);
		}

		// terrain mesh generation
		if (glResources.queries.scalarFieldGenerationTimeElapsedQuery.resultAvailable()) {
			glResources.queries.scalarFieldGenerationTimeElapsedQuery.getResult(state.timings.scalarFieldGenerationMs);
		}
		if (glResources.queries.terrainMeshGenerationTimeElapsedQuery.resultAvailable()) {
			glResources.queries.terrainMeshGenerationTimeElapsedQuery.getResult(state.timings.terrainMeshGenerationMs);
		}

		// rendering
		if (glResources.queries.mainRenderingTimeElapsedQuery.resultAvailable()) {
			glResources.queries.mainRenderingTimeElapsedQuery.getResult(state.timings.mainRenderingMs);
		}
		if (glResources.queries.spotLightsDebugRenderingTimeElapsedQuery.resultAvailable()) {
			glResources.queries.spotLightsDebugRenderingTimeElapsedQuery.getResult(state.timings.spotLightsDebugRenderingMs);
		}
		if (glResources.queries.pointLightsDebugRenderingTimeElapsedQuery.resultAvailable()) {
			glResources.queries.pointLightsDebugRenderingTimeElapsedQuery.getResult(state.timings.pointLightsDebugRenderingMs);
		}
		// maybe some of the results will never be available

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


// load some large and complicated model
// * Sponza Atrium (Crytek/Intel)
// * San Miguel
// * Bistro Exterior/Interior
// https://graphics.stanford.edu/data/3Dscanrep/
// * Stanford Dragon
// * Stanford Bunny
// * Stanford Buddha
// could load multiple and allow the user to switch between them
// could also use one of the PGR2 provided models
// TODO - figure out which model to display and how to pass it to the GPU (standard vertex arrays + VBO should suffice)

// generate N of each type of light on the CPU and buffer their values
//	might need to modify the structure of lights to also incorporate the values related to movement
//	or could have those in a separate buffer (maybe better)
// TODO

// loop:
//		update light positions/directions using a shader
// 
//		compute clusters using a shader
//		// TODO
// 
//		display lighting
//			* naive (going through all lights)
//			* clustered (going only through the relevant indices)
//		(maybe each light should have one small sphere/point assigned to it)
