#version 450
layout (binding = 1) uniform sampler texSampler;
layout (binding = 2) uniform texture2D textures;
layout (location = 0) in vec3 inColor;
layout (location = 1) in vec2 inUV;
layout (location = 0) out vec4 outFragColor;

vec4  addColor = vec4(inColor.r, inColor.g, inColor.b, 1);

//FOR BIGGER FONTS, USE HIGHER WIDTH, AND SMALLER EDGE like 0.51 and 0.02
//FOR SMALLER FONT, USE SMALLER WIDTH, AND HIGHER EDGE like 0.46 and 0.19
const float width = 0.2;
const float edge = 0.0;

void main()
{
	float distance = 1.0 - texture(sampler2D(textures, texSampler), inUV).a;
	vec3 rgb =  addColor.rgb;
	float alpha = 1.0 - smoothstep(width, width+edge, distance);
	outFragColor = vec4(rgb, alpha);
}