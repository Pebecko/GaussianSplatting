#version 460


#define RENDERING_SETTINGS_BUFFER_BINDING	0	// UBO

#define GAUSSIAN_SPLATS_BUFFER_BINDING		0	// SSBO


in vec3 color_v;

out vec4 color;


void main() {
	color = vec4(color_v, 1.0f);
}
