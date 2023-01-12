#version 330 core

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inRGB;
layout (location = 2) in vec2 inTexcoord;

out vec3 outRGB;
out vec2 outTexcoord;
uniform mat4 transform;

void main()
{
	outRGB = inRGB;
	outTexcoord = inTexcoord;
	gl_Position = transform * vec4(inPos, 1.0);
}