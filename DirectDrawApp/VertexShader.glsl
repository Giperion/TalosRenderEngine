#version 300
#pragma optimize (off)
#pragma debug (on)
layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec2 in_UV;

out vec2 testpos;
out vec2 testUV;
void main()
{
	testUV = in_UV;
	testpos = in_Position.rg;
	gl_Position = vec4(in_Position, 1.0);
}
