#version 300 es
layout(location = 0) in vec2 inputTextureCoordinate;

out vec2 textureCoordinate;
     
void main()
{
    gl_Position = vec4(inputTextureCoordinate*2.0-1.0,1.0,1.0);
    textureCoordinate = inputTextureCoordinate;
}