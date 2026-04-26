#include "glm/glm.hpp"
#include "glad/glad.h"


// deg = 0 -> constant color
// deg = 1 -> linear gradient (one side brighter)
// deg = 2 -> soft lighting, blobs, highlights (most common)
// deg >= 3 -> more detail, diminishing returns
#define SPHERICAL_HARMONICS_DEGREE 2
#define TOTAL_SPHERICAL_HARMONICS_COEFFICIENTS_PER_CHANNEL(degree) ((degree + 1) * (degree + 1))  // coefficients per channel: (deg + 1)^2


class SphericalHarmonics {
	// TODO - set the number of coefficients based on SPHERICAL_HARMONICS_MAX_DEGREE
	glm::vec3 coefficients[TOTAL_SPHERICAL_HARMONICS_COEFFICIENTS_PER_CHANNEL(SPHERICAL_HARMONICS_DEGREE)];
	//static GLfloat ...  // array of generated lambda function with precomputed spherical harmonics basis functions

	GLfloat evaluate(glm::vec3 direction) {
		glm::vec3 color = glm::vec3(0.0f);
	}
};


class GaussianSplat {
	public:
		GaussianSplat();
		~GaussianSplat();

	private:

	public:
		glm::vec3 position;  // world coordinates
		GLfloat opacity;  // blending coefficient (alpha) (TODO - find the right name)
		glm::mat3x3 covarianceMatrix;  // anisotropic covariance
		SphericalHarmonics sphericalHarmonics;
};