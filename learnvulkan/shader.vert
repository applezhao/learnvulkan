#version 450
#extension GL_ARB_separate_shader_objects : enable
out gl_PerVertex
{
	vec4 gl_Position;
};


layout(location = 0) out vec2 UVCoord;
layout(location = 1) out float Discard;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inPosNext;
layout(location = 2) in vec2 ExtrudeCode;

layout(set = 0, binding = 0) uniform UniformBufferObject
{
	mat4 model;
	mat4 view;
	mat4 proj;
	
	vec4 gLineColor;
	float gLineWidth;
	
	float gHalfFilterWidth;

}ubo;


void main()
{
	gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);

	float w = ceil( 1.25 + ubo.gLineWidth ) *0.5;

    //vertex 0
    if( ExtrudeCode.x == 0.0 )
    {
        UVCoord = vec2( 0, -w);
    }

    //vertex 1
    else if( ExtrudeCode.x == 1.0 )
    {
        UVCoord = vec2(0, -w);
		
    }
    //vertex 2
    else if( ExtrudeCode.x == 2.0 )
    {
        UVCoord = vec2( 1, w);
		
    }
    //vertex 3
    else if( ExtrudeCode.x == 3.0 )
    {
        UVCoord = vec2(1, w);
		
    }
}