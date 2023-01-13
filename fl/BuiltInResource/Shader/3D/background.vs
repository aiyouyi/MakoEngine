#version 300 es
layout(location = 0) in vec3 aPos;

out vec3 WorldPos;

uniform mat4 projection;
uniform mat4 view;
//uniform mat4 rotate;


void main()
{
    WorldPos = aPos;

	mat4 rotView = mat4(mat3(view));
	vec4 clipPos = projection * rotView *vec4(WorldPos, 1.0);
	gl_Position = clipPos.xyww;
	gl_Position.y = -gl_Position.y;  //由于背景图渲染出来相机是（0， -1， 0），所以需要上下翻转
}