#include <glfw/glfw3.h>


namespace demos::gui {
	bool init(GLFWwindow* window, const char* openglVersion);  // using the current set postInitFunction

	void newFrame();
	void renderFrame();  // consider exposing this method and removing it from updateFrame()

	bool destroy();	
}
