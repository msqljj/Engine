#version 450

layout (binding = 1) uniform sampler texSampler;

layout (binding = 2) uniform texture2D textures;

layout (location = 0) in vec3 inColor;

layout (location = 1) in vec2 inUV;

layout (location = 0) out vec4 outFragColor;

vec4  addColor = vec4(inColor.r, inColor.g, inColor.b, 1);
const vec2 shadowOffset = vec2(0.0,0.0); // Between 0 and spread / textureSize
const float shadowSmoothing = 0.5; // Between 0 and 0.5
const vec4 shadowColor = vec4(0,0,0,1); // R.G.B.A Between 0 and 1

void main()
{
	float distance = texture(sampler2D(textures, texSampler), inUV).a;
	float smoothing = fwidth(distance); 
    	float alpha = smoothstep(0.5 - smoothing, 0.5 + smoothing, distance);
    	vec4 text = vec4(addColor.rgb, addColor.a * alpha);
	vec2 tempUV = inUV;
	tempUV = tempUV-shadowOffset;
    	float shadowDistance = texture(sampler2D(textures, texSampler), tempUV).a;
    	float shadowAlpha = smoothstep(0.5 - shadowSmoothing, 0.5 + shadowSmoothing, shadowDistance);
    	vec4 shadow = vec4(shadowColor.rgb, shadowColor.a * shadowAlpha);
    	outFragColor = mix(shadow, text, text.a);
}
