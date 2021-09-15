#version 450

//doesnt need same name as out variable in vertex shader, as location identifies each veraible sent
layout(location = 0) in vec3 fragColour;

layout(location = 0) out vec4 outColour;

void main()
{
    outColour = vec4(fragColour, 1.0);
}