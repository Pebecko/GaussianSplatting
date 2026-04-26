#pragma once

#include "glad/glad.h"
#include "glm/glm.hpp"


struct GaussianSplat;


struct alignas(16) Point {
	Point();
	Point(const glm::vec3 position, const GLfloat radius, const glm::vec3 color, const GLfloat opacity);
	Point(const GaussianSplat& gaussianSplat);

	~Point() = default;

	glm::vec3 position = glm::vec3(0.0f);  // world coordinates
	GLfloat radius = 1.0f;
	glm::vec3 color = glm::vec3(0.0f);  // RGB
	GLfloat opacity = 1.0f;  // alpha
};
static_assert(sizeof(Point) == 32);
