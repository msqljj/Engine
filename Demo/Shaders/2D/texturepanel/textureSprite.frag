#version 450

layout (binding = 1) uniform sampler texSampler;
layout (binding = 2) uniform texture2D textures;
layout (location = 0) in vec3 inColor;
layout (location = 1) in vec2 inUV;

layout (location = 0) out vec4 outFragColor;

void main()
{
	//outFragColor = vec4(inColor, 0.5);
	outFragColor = texture(sampler2D(textures, texSampler) , inUV);
}