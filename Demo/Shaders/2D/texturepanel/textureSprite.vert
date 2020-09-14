#version 450

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUV;
layout (location = 3) in vec3 inColor;

layout (binding = 0) uniform UBO
{
	mat4 modelMatrix;
	mat4 projectionMatrix;
} ubo;

layout (location = 0) out vec3 outColor;
layout (location = 1) out vec2 outUV;
out gl_PerVertex 
{
    vec4 gl_Position;   
};

void main() 
{
	outColor = inColor;
	outUV = inUV;
	gl_Position = ubo.projectionMatrix * ubo.modelMatrix * vec4(inPos.xyz, 1.0);
}
