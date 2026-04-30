#include "gaussianSplatting/primitives/point.hpp"
#include "gaussianSplatting/primitives/gaussianSplat.hpp"
#include "gaussianSplatting/randomAttributes.hpp"


Point::Point() {
    static std::mt19937 rng{std::random_device{}()};

    //this->position = randVec3(rng, glm::vec3(-1.0f), glm::vec3(1.0f));
    this->radius = randScalar(rng, 0.1f, 1.0f);
    this->color = randVec3(rng, glm::vec3(0.0f), glm::vec3(1.0f));
    this->opacity = randScalar(rng, 0.0f, 1.0f);
}

Point::Point(const glm::vec3 position, const GLfloat radius, const glm::vec3 color, const GLfloat opacity) : position(position), radius(radius), color(color), opacity(opacity) {

}

Point::Point(const GaussianSplat& gaussianSplat) {
    this->position = gaussianSplat.position;
    this->radius = 1.0f;
    this->color = gaussianSplat.sphericalHarmonics.color();
    this->opacity = gaussianSplat.opacity;
};