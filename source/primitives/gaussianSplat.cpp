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


glm::vec3 SphericalHarmonics::evaluate(glm::vec3 viewDirection) const {
    // TODO - check these constants

    viewDirection = glm::normalize(viewDirection);

    glm::vec3 color(0.0f);

    // l = 0
    color += glm::vec3(this->coefficients[0].x, this->coefficients[0].y, this->coefficients[0].z) * SPHERICAL_HARMONICS_DEGREE_0_CONSTANT_0;

    // l = 1
    color += glm::vec3(this->coefficients[1].x, this->coefficients[1].y, this->coefficients[1].z) * (-SPHERICAL_HARMONICS_DEGREE_1_CONSTANT_0) * viewDirection.y;
    color += glm::vec3(this->coefficients[2].x, this->coefficients[2].y, this->coefficients[2].z) * SPHERICAL_HARMONICS_DEGREE_1_CONSTANT_0 * viewDirection.z;
    color += glm::vec3(this->coefficients[3].x, this->coefficients[3].y, this->coefficients[3].z) * (-SPHERICAL_HARMONICS_DEGREE_1_CONSTANT_0) * viewDirection.x;

    // l = 2
    color += glm::vec3(this->coefficients[4].x, this->coefficients[4].y, this->coefficients[4].z) * SPHERICAL_HARMONICS_DEGREE_2_CONSTANT_0 * viewDirection.x * viewDirection.y;
    color += glm::vec3(this->coefficients[5].x, this->coefficients[5].y, this->coefficients[5].z) * SPHERICAL_HARMONICS_DEGREE_2_CONSTANT_1 * viewDirection.y * viewDirection.z;
    color += glm::vec3(this->coefficients[6].x, this->coefficients[6].y, this->coefficients[6].z) * SPHERICAL_HARMONICS_DEGREE_2_CONSTANT_2 * (3.0f * viewDirection.z * viewDirection.z - 1.0f);
    color += glm::vec3(this->coefficients[7].x, this->coefficients[7].y, this->coefficients[7].z) * SPHERICAL_HARMONICS_DEGREE_2_CONSTANT_3 * viewDirection.x * viewDirection.z;
    color += glm::vec3(this->coefficients[8].x, this->coefficients[8].y, this->coefficients[8].z) * SPHERICAL_HARMONICS_DEGREE_2_CONSTANT_4 * (viewDirection.x * viewDirection.x - viewDirection.y * viewDirection.y);

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
