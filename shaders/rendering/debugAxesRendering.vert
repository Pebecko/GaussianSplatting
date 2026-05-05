#version 460


#define RENDERING_SETTINGS_BUFFER_BINDING	0	// UBO

#define GAUSSIAN_SPLATS_BUFFER_BINDING		0	// SSBO


layout (std140, binding = RENDERING_SETTINGS_BUFFER_BINDING) uniform RenderingSettings {
    mat4x4 projectionMatrix;
    mat4x4 viewMatrix;

	float cameraNearDistance;
	float cameraFarDistance;

	int screenWidth;
	int screenHeight;

	float basePointRadius;
	float baseLightRadius;

	float _pad0[2];
} renderingSettings;


const vec3 axesEndpoints[3][2] = {
	{vec3(0.0f, 0.0f, 0.0f), vec3(1.0f, 0.0f, 0.0f)},
	{vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f)},
	{vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 0.0f, 1.0f)},
};

const vec3 axesColors[3] = {
	vec3(1.0f, 0.0f, 0.0f),
	vec3(0.0f, 1.0f, 0.0f),
	vec3(0.0f, 0.0f, 1.0f),
};


out vec3 color_v;


void main() {
	gl_Position = renderingSettings.projectionMatrix * renderingSettings.viewMatrix * vec4(axesEndpoints[gl_InstanceID][gl_VertexID], 1.0f);
	color_v = axesColors[gl_InstanceID];
}
