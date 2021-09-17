#version 450

layout(push_constant) uniform fragconstants
{
    layout(offset = 64)vec4 colour;
    uint texID;
} pc;

layout(set = 1, binding = 0) uniform sampler texSamp;
layout(set = 1, binding = 1) uniform texture2D textures[100];

layout(location = 0) in vec2 texCoord;
layout(location = 0) out vec4 outColour;

void main()
{
    outColour = texture(sampler2D(textures[pc.texID], texSamp), texCoord);
}