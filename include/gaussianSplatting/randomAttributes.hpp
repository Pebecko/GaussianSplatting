#pragma once

#include <random>

#include "glm/glm.hpp"


inline float randScalar(std::mt19937& rng, float min, float max) {
    std::uniform_real_distribution<float> dist(min, max);
    return dist(rng);
}

inline glm::vec3 randVec3(std::mt19937& rng, glm::vec3 min, glm::vec3 max) {
    return {
        randScalar(rng, min.x, max.x),
        randScalar(rng, min.y, max.y),
        randScalar(rng, min.z, max.z)
    };
}

inline glm::vec3 randUnitVec3(std::mt19937& rng) {
    glm::vec3 vec;
    do {
        vec = randVec3(rng, glm::vec3(-1.0f), glm::vec3(1.0f));
    }
    while (glm::dot(vec, vec) < 0.0001f);

    return glm::normalize(vec);
}

inline glm::vec3 randNonParallelUnitVec3(std::mt19937& rng, glm::vec3 vector) {
    glm::vec3 nonParallelVector;
    do {
        nonParallelVector = randVec3(rng, glm::vec3(-1.0f), glm::vec3(1.0f));
    }
    while (glm::abs(glm::dot(glm::normalize(vector), glm::normalize(nonParallelVector))) > 0.0001f);

    return glm::normalize(nonParallelVector);
}
