#version 450

layout(push_constant) uniform constants
{
    mat4 model;
    vec3 colour;
} pcs;

layout(binding = 0) uniform UniformBufferObject
{
    mat4 view;
    mat4 proj;
} ubo;


layout(location = 0) in vec2 inPos;
layout(location = 1) in vec2 texPos;

layout(location = 0) out vec3 fragColour;

void main()
{
    gl_Position = ubo.proj * ubo.view * pcs.model
     * vec4(inPos, 0, 1);
    fragColour = pcs.colour;
}