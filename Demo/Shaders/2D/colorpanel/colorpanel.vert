#version 450

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec4 inColor;

layout (binding = 0) uniform UBO
{
	mat4 modelMatrix;
	mat4 projectionMatrix;
} ubo;

layout (location = 0) out vec4 outColor;

out gl_PerVertex 
{
    vec4 gl_Position;   
};


void main() 
{
	outColor = inColor;
	gl_Position = ubo.projectionMatrix * ubo.modelMatrix * vec4(inPos.xyz, 1.0);
}
