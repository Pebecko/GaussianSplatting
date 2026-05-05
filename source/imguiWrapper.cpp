#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <implot/implot.h>

#include "gaussianSplatting/imguiWrapper.hpp"


bool demos::gui::init(GLFWwindow* window, const char* openglVersion) {
	// TODO - check that ImGui and ImPlot have not been initialized

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();  // creating ImGui context
	ImPlot::CreateContext();  // creating ImPlot context

	// setting up platform/renderer backends
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 460");

	return true;
}


void demos::gui::newFrame() {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}


void demos::gui::renderFrame() {
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}


bool demos::gui::destroy() {
	// TODO - check that ImGui and ImPlot contexts have been created

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImPlot::DestroyContext();
	ImGui::DestroyContext();

	return true;
}


// void defaultPostInitFunction() {
// 	ImGuiIO& io = ImGui::GetIO();
// 
// 	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;  // enable keyboard controls
// 	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;  // enable docking
// 
// 	// ImGuiStyle& style = ImGui::GetStyle();
// 	// style.FontSizeBase = 20.0f;
// 
// 	// load fonts
// 	// ImFontConfig fontConfig;
// 	// ImFont* defaultFont = io.Fonts->AddFontDefault(&fontConfig);
// 	// IM_ASSERT(defaultFont != nullptr);
// 	// ImFont* additionalFont = io.Fonts->AddFontFromFileTTF("pathToFont.ttf");
// 	// IM_ASSERT(additionalFont != nullptr);
// 	// use using PushFont()/PopFont()
// 
// 	io.IniFilename = nullptr;  // turns off ui persistence
// 
// 	ImGui::StyleColorsDark();
// }
