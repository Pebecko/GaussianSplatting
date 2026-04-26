#include "gaussianSplatting/primitives/gaussianSplat.hpp"
#include "gaussianSplatting/primitives/point.hpp"
#include "gaussianSplatting/randomAttributes.hpp


// TODO
SphericalHarmonics::SphericalHarmonics() { 

}

// TODO
SphericalHarmonics::SphericalHarmonics(const glm::vec3 color) {

}

// TODO
SphericalHarmonics::SphericalHarmonics(... coefficients) {

}


// TODO
GLfloat SphericalHarmonics::evaluate(const glm::vec3 direction) {

}

// TODO
glm::vec3 SphericalHarmonics::color() {

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

GaussianSplat::GaussianSplat(const Point& point) {
    this->position = point.position;
    this->opacity = point.opacity;
    this->covarianceRotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    this->covarianceScale = glm::vec3(point.radius);
    this->sphericalHarmonics = SphericalHarmonics(point.color);
}

glm::mat3x3 GaussianSplat::covarianceMatrix() {
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
