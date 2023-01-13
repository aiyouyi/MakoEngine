#version 300 es
layout(location = 0) in vec2 a_position;
layout(location = 1) in vec2 a_texcoord;

out vec2 textureCoordinate;
out vec2 textureCoordinate2;


void main()
{
        gl_Position = vec4(a_position,0.5,1.0);
        textureCoordinate2 = (a_position.xy+vec2(1.0, 1.0))*0.5;
        textureCoordinate = a_texcoord;
}
