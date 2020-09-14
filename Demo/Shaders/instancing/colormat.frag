#version 450

layout (location = 0) in vec3 inNormal;
layout (location = 1) in vec3 inColor;
layout (location = 2) in vec2 inUV;
layout (location = 3) in vec3 inViewVec;
layout (location = 4) in vec3 inLightVec;

//Material struct
layout (binding = 1) uniform Material {
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	float opacity;
} material;

layout (location = 0) out vec4 outFragColor;

void main ()
{
	vec4 color = vec4(inColor, 1.0);
	vec3 normal = normalize(inNormal);
	vec3 lightVector = normalize(inLightVec);
	vec3 viewVector = normalize(inViewVec);
	vec3 reflectVector = reflect(-lightVector, normal);

	vec3 diffuse = max(dot(normal, lightVector), 0.0) * material.diffuse.rgb;
	vec3 specular = pow(max(dot(reflectVector, viewVector), 0.0), 16.0) * material.specular.rgb;

	outFragColor = vec4(diffuse * color.rgb + specular, 1.0);
}