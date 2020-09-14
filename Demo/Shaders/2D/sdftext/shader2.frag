#version 450
layout (binding = 1) uniform sampler texSampler;
layout (binding = 2) uniform texture2D textures;
layout (location = 0) in vec3 inColor;
layout (location = 1) in vec2 inUV;
layout (location = 0) out vec4 outFragColor;

vec4  addColor = vec4(inColor.r, inColor.g, inColor.b, 1);
const float smoothing = 0.25 / (4*1.0);

void main()
{
	float distance = 1.0 - texture(sampler2D(textures, texSampler), inUV).a;
	float smoothing = fwidth(distance); 
	float alpha = 1.0 - smoothstep(0.5 - smoothing, 0.5 + smoothing, distance);
	vec3 rgb =  addColor.rgb;
	outFragColor = vec4(rgb, alpha);
}