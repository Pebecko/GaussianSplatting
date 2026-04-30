#pragma once

#include <span>

#include "glad/glad.h"
#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"


// deg = 0 -> constant color
// deg = 1 -> linear gradient (one side brighter)
// deg = 2 -> soft lighting, blobs, highlights (most common)
// deg >= 3 -> more detail, diminishing returns
constexpr int SPHERICAL_HARMONICS_DEGREE = 2;
constexpr int SPHERICAL_HARMONICS_COEFFICIENTS_PER_CHANNEL = (SPHERICAL_HARMONICS_DEGREE + 1) * (SPHERICAL_HARMONICS_DEGREE + 1);  // coefficients per channel: (deg + 1)^2
// constexpr int SPHERICAL_HARMONICS_TOTAL_COEFFICIENTS = 3 * SPHERICAL_HARMONICS_COEFFICIENTS_PER_CHANNEL;  // total coefficients: 3 * (deg + 1)^2

constexpr float SPHERICAL_HARMONICS_DEGREE_0_CONSTANT_0 = 0.28209479177387814f;
constexpr float SPHERICAL_HARMONICS_DEGREE_1_CONSTANT_0 = 0.4886025119029199f;
constexpr float SPHERICAL_HARMONICS_DEGREE_2_CONSTANT_0 = 1.0925484305920792f;
constexpr float SPHERICAL_HARMONICS_DEGREE_2_CONSTANT_1 = -1.0925484305920792f;
constexpr float SPHERICAL_HARMONICS_DEGREE_2_CONSTANT_2 = 0.31539156525252005f;
constexpr float SPHERICAL_HARMONICS_DEGREE_2_CONSTANT_3 = -1.0925484305920792f;
constexpr float SPHERICAL_HARMONICS_DEGREE_2_CONSTANT_4 = 0.5462742152960396f;

// TODO - figure out how to represent spherical harmonics in memory
struct alignas(16) SphericalHarmonics {
	SphericalHarmonics();
	SphericalHarmonics(const glm::vec3 color);
	SphericalHarmonics(std::span<glm::vec4> coefficients);

	~SphericalHarmonics() = default;
	
	glm::vec3 evaluate(glm::vec3 direction) const;  // TODO - find the coefficient constants necessary for evaluation
	glm::vec3 color() const;

	// vec4 because of the GPU padding (stored as {chan_i_coeff_0, chan_i_coeff_1, chan_i_coeff_2, pad})
	glm::vec4 coefficients[SPHERICAL_HARMONICS_COEFFICIENTS_PER_CHANNEL] = {{0.0f, 0.0f, 0.0f, 0.0f}};	
};
static_assert(sizeof(SphericalHarmonics) == 144);

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

	glm::mat3x3 covarianceMatrix() const;  // anisotropic covariance

	glm::vec3 position = glm::vec3(0.0f);  // world coordinates
	GLfloat opacity = 1.0f;  // blending coefficient (alpha)

	glm::quat covarianceRotation = glm::fquat(1.0f, 0.0f, 0.0f, 0.0f);  // axes orientation
	glm::vec3 covarianceScale = glm::vec3(1.0f);  // standard deviation along axes
	GLfloat _pad0 = 0.0f;

	SphericalHarmonics sphericalHarmonics;
};
static_assert(sizeof(GaussianSplat) == sizeof(SphericalHarmonics) + 48);
