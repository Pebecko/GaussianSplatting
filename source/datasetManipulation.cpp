#include <vector>
#include <filesystem>
#include <fstream>

#include "glad/glad.h"
#include "assimp/Bitmap.h"
#include "glm/glm.hpp"
#include "tinyply.h"

#include "gaussianSplatting/primitives/point.hpp"
#include "gaussianSplatting/primitives/gaussianSplat.hpp"


std::vector<Point> loadPointCloud(const std::filesystem::path& filepath) {
	std::fstream file(filepath, std::ios::in | std::ios::binary);
	tinyply::PlyFile ply;

	ply.parse_header(file);

    std::shared_ptr<tinyply::PlyData> positions;
    std::shared_ptr<tinyply::PlyData> colors;

    // position - required, 3 * float32
    positions = ply.request_properties_from_element("vertex", {"x", "y", "z"});

    // radius - optional, 1 * float32
    // TODO

    // color - optional, 3 * uint8
    try {
        colors = ply.request_properties_from_element("vertex", {"red", "green", "blue"});
    }
    catch (const std::exception& exception) {
        colors = nullptr;
    }

    // opacity - optional, 1 * float32

    ply.read(file);

    const size_t totalPositions = positions->count;
    const float* positionsData = reinterpret_cast<const float*>(positions->buffer.get());


    const size_t totalColors = colors != nullptr ? colors->count : 0u;
    const uint8_t* colorsData = colors != nullptr ? reinterpret_cast<const uint8_t*>(colors->buffer.get()) : nullptr;

    std::vector<Point> points;
    points.reserve(totalPositions);

    glm::vec3 position, color;

    for (size_t pointIndex = 0; pointIndex < totalPositions; pointIndex++) {
        position = glm::vec3(
            positionsData[3 * pointIndex + 0],
            positionsData[3 * pointIndex + 1],
            positionsData[3 * pointIndex + 2]
        );

        if (totalColors >= totalPositions) {
            color = glm::vec3(
                colorsData[3 * pointIndex + 0] / 255.0f,
                colorsData[3 * pointIndex + 1] / 255.0f,
                colorsData[3 * pointIndex + 2] / 255.0f
            );
        }
        else if (totalColors >= 1u) {
            color = glm::vec3(
                colorsData[0] / 255.0f,
                colorsData[1] / 255.0f,
                colorsData[2] / 255.0f
            );
        }
        else {
            color = glm::vec3(0.5f, 0.0f, 0.5f);  // default color (purple)
        }

        Point point(position, 1.0f, color, 1.0f);

        points.push_back(point);
    }

    return points;
}

//std::vector<GaussianSplat> loadGaussianSplatCloud(const std::filesystem::path& filepath) {
//
//}


void savePointCloud(std::vector<Point> pointCloud, const std::filesystem::path& filepath) {

}

//void saveGaussianSplatCloud(std::vector<GaussianSplat> gaussianSplatCloud, const std::filesystem::path& filepath) {
//    tinyply::PlyFile ply;
//
//    std::vector<float> positions;
//    std::vector<float> scales;
//    std::vector<float> rotations;
//    std::vector<float> colors;
//    std::vector<float> opacity;
//
//    for (const auto& s : gaussianSplatCloud) {
//        positions.insert(positions.end(), {s.position.x, s.position.y, s.position.z});
//        scales.insert(scales.end(), {s.scale.x, s.scale.y, s.scale.z});
//        rotations.insert(rotations.end(), {s.rotation.x, s.rotation.y, s.rotation.z, s.rotation.w});
//        colors.insert(colors.end(), {s.color.r, s.color.g, s.color.b});
//        opacity.push_back(s.opacity);
//    }
//
//    ply.add_properties_to_element("vertex", {"x","y","z"},
//        tinyply::Type::FLOAT32, gaussianSplatCloud.size(),
//        reinterpret_cast<uint8_t*>(positions.data()), Type::INVALID, 0);
//
//    ply.add_properties_to_element("vertex", {"scale_x","scale_y","scale_z"},
//        Type::FLOAT32, splats.size(),
//        reinterpret_cast<uint8_t*>(scales.data()), Type::INVALID, 0);
//
//    ply.add_properties_to_element("vertex", {"rot_x","rot_y","rot_z","rot_w"},
//        Type::FLOAT32, splats.size(),
//        reinterpret_cast<uint8_t*>(rotations.data()), Type::INVALID, 0);
//
//    ply.add_properties_to_element("vertex", {"red","green","blue"},
//        Type::FLOAT32, splats.size(),
//        reinterpret_cast<uint8_t*>(colors.data()), Type::INVALID, 0);
//
//    ply.add_properties_to_element("vertex", {"opacity"},
//        Type::FLOAT32, splats.size(),
//        reinterpret_cast<uint8_t*>(opacity.data()), Type::INVALID, 0);
//
//    std::filebuf fb;
//    fb.open(path, std::ios::out | std::ios::binary);
//    std::ostream outstream(&fb);
//
//    ply.write(outstream, true); // true = binary
//}


// turn point cloud into splat cloud and back
// save point cloud/splat cloud
