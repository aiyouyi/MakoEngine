#version 300 es
layout(location = 0) in  vec3 a_position;
out vec2 textureCoordinate;


void main()
{
	gl_Position = vec4(a_position,1.0);
	textureCoordinate = a_position.xy*0.5+0.5;
}