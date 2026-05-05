#version 460


#define GENERATION_SETTINGS_BUFFER_BINDING					0	// UBO
#define RENDERING_SETTINGS_BUFFER_BINDING					1	// UBO
#define LIGHTING_PARAMETERS_BUFFER_BINDING					2	// UBO
#define MATERIALS_BUFFER_BINDING							3	// UBO

#define CHUNKS_DISPATCH_BUFFER_BINDING						0	// SSBO
#define INDIRECT_DRAW_COMMAND_BUFFER_BINDING				1	// SSBO
#define NOISE_VALUES_BUFFER_BINDING							2	// SSBO
#define MESH_TRIANGLE_COUNTS_BUFFER_BINDING					3	// SSBO
#define MESH_VERTICES_BUFFER_BINDING						4	// SSBO
#define MESH_NORMALS_BUFFER_BINDING							5	// SSBO
#define DIRECTIONAL_LIGHTS_BUFFER_BINDING					6	// SSBO
#define SPOT_LIGHTS_BUFFER_BINDING							7	// SSBO
#define POINT_LIGHTS_BUFFER_BINDING							8	// SSBO
#define CLUSTER_SPOT_LIGHT_COUNTS_BUFFER_BINDING			9	// SSBO
#define CLUSTER_SPOT_LIGHT_INDICES_OFFSETS_BUFFER_BINDING	10	// SSBO
#define CLUSTER_SPOT_LIGHT_INDICES_BUFFER_BINDING			11	// SSBO
#define CLUSTER_POINT_LIGHT_COUNTS_BUFFER_BINDING			12	// SSBO
#define CLUSTER_POINT_LIGHT_INDICES_OFFSETS_BUFFER_BINDING	13	// SSBO
#define CLUSTER_POINT_LIGHT_INDICES_BUFFER_BINDING			14	// SSBO

#define DIRECTIONAL_LIGHT_ORBITS_BUFFER_BINDING				15	// SSBO
#define SPOT_LIGHT_ORBITS_BUFFER_BINDING					16	// SSBO
#define POINT_LIGHT_ORBITS_BUFFER_BINDING					17	// SSBO


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
