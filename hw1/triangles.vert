#version 430 core

layout(location = 0) in vec4 vPosition;
layout(location = 1) in vec3 vColor;

// Defining a default color vector.
out vec3 Color;

void
main()
{
	gl_Position = vPosition;
	Color = vColor;
}
