#include "gaussianSplatting/primitives/gaussianSplat.hpp"
#include "gaussianSplatting/primitives/point.hpp"
#include "gaussianSplatting/randomAttributes.hpp"


SphericalHarmonics::SphericalHarmonics() { 
    for (glm::vec4& coefficient : this->coefficients) {
        coefficient = glm::vec4(0.0f);
    }
}

SphericalHarmonics::SphericalHarmonics(const glm::vec3 color) {
    for (glm::vec4& coefficient : this->coefficients) {
        coefficient = glm::vec4(0.0f);
    }

    this->coefficients[0] = glm::vec4(color / SPHERICAL_HARMONICS_DEGREE_0_CONSTANT_0, 0.0f);
}

SphericalHarmonics::SphericalHarmonics(std::span<glm::vec4> coefficients) {
    if (coefficients.size() != SPHERICAL_HARMONICS_COEFFICIENTS_PER_CHANNEL) {
        throw std::runtime_error("Invalid SPHERICAL_HARMONICS coefficient count");
    }

    std::copy(coefficients.begin(), coefficients.end(), this->coefficients);
}


glm::vec3 SphericalHarmonics::evaluate(glm::vec3 direction) const {
    // TODO - check these constants

    direction = glm::normalize(direction);

    glm::vec3 color(0.0f);

    // l = 0
    color += this->coefficients[0] * SPHERICAL_HARMONICS_DEGREE_0_CONSTANT_0;

    // l = 1
    color += this->coefficients[1] * (-SPHERICAL_HARMONICS_DEGREE_1_CONSTANT_0) * direction.y;
    color += this->coefficients[2] * SPHERICAL_HARMONICS_DEGREE_1_CONSTANT_0 * direction.z;
    color += this->coefficients[3] * (-SPHERICAL_HARMONICS_DEGREE_1_CONSTANT_0) * direction.x;

    // l = 2
    color += this->coefficients[4] * SPHERICAL_HARMONICS_DEGREE_2_CONSTANT_0 * direction.x * direction.y;
    color += this->coefficients[5] * SPHERICAL_HARMONICS_DEGREE_2_CONSTANT_1 * direction.y * direction.z;
    color += this->coefficients[6] * SPHERICAL_HARMONICS_DEGREE_2_CONSTANT_2 * (3.0f * direction.z * direction.z - 1.0f);
    color += this->coefficients[7] * SPHERICAL_HARMONICS_DEGREE_2_CONSTANT_3 * direction.x * direction.z;
    color += this->coefficients[8] * SPHERICAL_HARMONICS_DEGREE_2_CONSTANT_4 * (direction.x * direction.x - direction.y * direction.y);

    return color;
}

glm::vec3 SphericalHarmonics::color() const {
    return SPHERICAL_HARMONICS_DEGREE_0_CONSTANT_0 * glm::vec3(coefficients[0]);
}


GaussianSplat::GaussianSplat() {
    static std::mt19937 rng{std::random_device{}()};

    // this->position = randVec3(rng, glm::vec3(-1.0f), glm::vec3(1.0f));
    this->opacity = randScalar(rng, 0.0f, 1.0f);
    this->covarianceRotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);  // TODO - randomize (maybe random angle + axis -> convert)
    this->covarianceScale = randVec3(rng, glm::vec3(0.1f), glm::vec3(1.0f));
    this->sphericalHarmonics = SphericalHarmonics(randVec3(rng, glm::vec3(0.0f), glm::vec3(1.0f)));
}


GaussianSplat::GaussianSplat(
    const glm::vec3 position,
    const GLfloat opacity,
    const glm::quat covarianceRotation,
    const glm::vec3 covarianceScale,
    const SphericalHarmonics& sphericalHarmonics
) : position(position), opacity(opacity), covarianceRotation(covarianceRotation), covarianceScale(covarianceScale), sphericalHarmonics(sphericalHarmonics) {

}

GaussianSplat::GaussianSplat(const Point& point) : position(point.position), opacity(point.opacity), covarianceRotation(glm::quat(1.0f, 0.0f, 0.0f, 0.0f)), covarianceScale(glm::vec3(point.radius)), sphericalHarmonics(SphericalHarmonics(point.color)) {

}

glm::mat3x3 GaussianSplat::covarianceMatrix() const {
    // covariance = R * diag(scale^2) * R^T

    glm::mat3x3 rotationMatrix = glm::mat3_cast(this->covarianceRotation);

    glm::vec3 scaleSquared = this->covarianceScale * this->covarianceScale;

    glm::mat3x3 scaleMatrix = glm::mat3x3(
        scaleSquared.x, 0.0f, 0.0f,
        0.0f, scaleSquared.y, 0.0f,
        0.0f, 0.0f, scaleSquared.z
    );

    return rotationMatrix * scaleMatrix * glm::transpose(rotationMatrix);
}
