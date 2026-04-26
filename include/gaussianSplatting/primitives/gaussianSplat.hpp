#pragma once

#include "glm/glm.hpp"
#include "glad/glad.h"


// deg = 0 -> constant color
// deg = 1 -> linear gradient (one side brighter)
// deg = 2 -> soft lighting, blobs, highlights (most common)
// deg >= 3 -> more detail, diminishing returns
#define SPHERICAL_HARMONICS_DEGREE 2
#define TOTAL_SPHERICAL_HARMONICS_COEFFICIENTS_PER_CHANNEL(degree) ((degree + 1) * (degree + 1))  // coefficients per channel: (deg + 1)^2


// TODO - figure out how to represent spherical harmonics in memory
struct alignas(16) SphericalHarmonics {
	SphericalHarmonics();
	SphericalHarmonics(const glm::vec3 color);
	SphericalHarmonics(... coefficients);

	~SphericalHarmonics() = default;
	
	GLfloat evaluate(const glm::vec3 direction);  // TODO - find the coefficient constants necessary for evaluation
	glm::vec3 color();

	// TODO - set the number of coefficients based on SPHERICAL_HARMONICS_MAX_DEGREE
	glm::vec3 coefficients[TOTAL_SPHERICAL_HARMONICS_COEFFICIENTS_PER_CHANNEL(SPHERICAL_HARMONICS_DEGREE)];
	//static GLfloat ...  // array of generated lambda function with precomputed spherical harmonics basis functions

	
};
static_assert(sizeof(SphericalHarmonics) == 32);


struct Point;

struct alignas(16) GaussianSplat {
	GaussianSplat();
	GaussianSplat(
		const glm::vec3 position,
		const GLfloat opacity,
		const glm::quat covarianceRotation,
		const glm::vec3 covarianceScale,
		const SphericalHarmonics& sphericalHarmonics
	);
	GaussianSplat(const Point& point);

	~GaussianSplat() = default;

	glm::mat3x3 covarianceMatrix();  // anisotropic covariance

	glm::vec3 position = glm::vec3(0.0f);  // world coordinates
	GLfloat opacity = 1.0f;  // blending coefficient (alpha)

	glm::quat covarianceRotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);  // axes orientation
	glm::vec3 covarianceScale = glm::vec3(1.0f);  // standard deviation along axes
	GLfloat _pad0;

	SphericalHarmonics sphericalHarmonics;
};
static_assert(sizeof(GaussianSplat) == sizeof(SphericalHarmonics) + 48);
