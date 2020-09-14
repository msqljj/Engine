#version 450
layout (binding = 1) uniform sampler texSampler;
layout (binding = 2) uniform texture2D textures;
layout (location = 0) in vec3 inColor;
layout (location = 1) in vec2 inUV;
layout (location = 0) out vec4 outFragColor;

vec4  addColor = vec4(inColor.r, inColor.g, inColor.b, 1);
const float outlineDistance = 0.4;  //Between 0 and 0.5, 0 = thick outline, 0.5 = no outline
const vec4 outlineColor = vec4(0,1,0,1); //Color of your choice 
const float smoothing = 0.25 / (4*1.0);

void main()
{
	float distance = texture(sampler2D(textures, texSampler), inUV).a;
	float outlineFactor = smoothstep(0.5 - smoothing, 0.5 + smoothing, distance);
    	vec4 color = mix(outlineColor, addColor, outlineFactor);
    	float alpha = smoothstep(outlineDistance - smoothing, outlineDistance + smoothing, distance);
    	outFragColor = vec4(color.rgb, color.a * alpha);

}
