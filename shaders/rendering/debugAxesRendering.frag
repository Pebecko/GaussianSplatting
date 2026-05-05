#version 460


in vec3 color_v;

out vec4 color;


void main() {
	color = vec4(color_v, 1.0f);
}
