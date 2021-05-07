#version 310 es
precision mediump float;

layout(binding = 0) uniform sampler2D texture0;

layout(location = 0) in vec4 vColor;
layout(location = 1) in vec2 vTexCoord;

layout(location = 0) out vec4 FragColor;

void main()
{
	FragColor = vColor * texture(texture0, vTexCoord);
}