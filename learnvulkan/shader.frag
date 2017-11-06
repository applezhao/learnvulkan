#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec2 UVCoord;
layout(location = 1) in float Discard;

layout(binding = 1) uniform sampler2D texSampler;

layout(set = 0, binding = 0) uniform UniformBufferObject
{
	mat4 model;
	mat4 view;
	mat4 proj;
	
	vec4 gLineColor;
	float gLineWidth;
	
	float gHalfFilterWidth;

}ubo;

layout(location = 0) out vec4 outColor;

void main()
{
	//outColor = ubo.gLineColor ;//texture(texSampler, fragTexCoord);
	
	if(Discard == 1.0)
	{
		discard;
	}

    

	float border = ubo.gLineWidth * 0.5 - ubo.gHalfFilterWidth;

	
	float dist = abs(UVCoord.y);

	dist -= border;


    if( dist < 0.0 )
    {
        outColor = ubo.gLineColor;
    }
    else //AA
    {
        dist /= ubo.gHalfFilterWidth;
        outColor = vec4(ubo.gLineColor.rgb, exp(-dist*dist) * ubo.gLineColor.a);
    }
}