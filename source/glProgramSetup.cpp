#include <fstream>

#include "gaussianSplatting/glProgramSetup.hpp"


// try {
//     std::string shaderSource = readShaderFromFile("shaders/terrain.frag");  // might be necessary to convert to std::filesystem::path
// }
// catch (const std::exception& exception) {
//     // ...
// }
// const char* shaderSourceArray = shaderSource.c_str();
// glShaderSource(shader, 1, &shaderSourceArray, nullptr);
std::string readShaderFromFile(const std::filesystem::path& shaderFilepath) {
	std::fstream file(shaderFilepath, std::ios::in | std::ios::binary);
	if (!file) {
		throw std::runtime_error("Failed to open shader file: " + shaderFilepath.string());
	}

	std::ostringstream buffer;
	buffer << file.rdbuf();

	return buffer.str();
}


gl::Program setupComputeProgram(const std::filesystem::path& computeShaderSource) {
	gl::Shader<GL_COMPUTE_SHADER> computeShader;
	if (!computeShader.compileFromSource(readShaderFromFile(computeShaderSource))) {
		// shader compilation failed
		throw std::runtime_error("Failed to compile shader from file: " + computeShaderSource.string());
	}

	gl::Program program = gl::Program();
	program.attachShader(computeShader);
	if (!program.link()) {
		// program linking failed
		throw std::runtime_error("Failed to link program from file: " + computeShaderSource.string());
	}

	return program;
}

gl::Program setupRenderingProgram(const std::filesystem::path& vertexShaderSource, const std::filesystem::path& fragmentShaderSource) {
	gl::Shader<GL_VERTEX_SHADER> vertexShader;
	if (!vertexShader.compileFromSource(readShaderFromFile(vertexShaderSource))) {
		// shader compilation failed
		throw std::runtime_error("Failed to compile shader from file: " + vertexShaderSource.string());
	}

	gl::Shader<GL_FRAGMENT_SHADER> fragmentShader;
	if (!fragmentShader.compileFromSource(readShaderFromFile(fragmentShaderSource))) {
		// shader compilation failed
		throw std::runtime_error("Failed to compile shader from file: " + fragmentShaderSource.string());
	}

	gl::Program program = gl::Program();
	program.attachShader(vertexShader);
	program.attachShader(fragmentShader);
	if (!program.link()) {
		// program linking failed
		throw std::runtime_error("Failed to link program from files:\n\t" + vertexShaderSource.string() + "\n\t" + fragmentShaderSource.string());
	}

	return program;
}
