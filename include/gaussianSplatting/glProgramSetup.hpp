#pragma once

#include <string>
#include <filesystem>

#include "glWrappers/program.hpp"
#include "glWrappers/shader.hpp"


std::string readShaderFromFile(const std::filesystem::path& shaderFilepath);

gl::Program setupComputeProgram(const std::filesystem::path& computeShaderSource);
gl::Program setupRenderingProgram(const std::filesystem::path& vertexShaderSource, const std::filesystem::path& fragmentShaderSource);
