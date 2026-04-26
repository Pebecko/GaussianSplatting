#include "glm/glm.hpp"


class Point {
	public:
		Point();
		~Point();

	private:
		
	public:
		glm::vec3 position;  // world coordinates
		glm::vec4 color;  // RGBA
};
