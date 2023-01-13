attribute vec3 position;
attribute vec2 inputTextureCoordinate;
varying vec2 textureCoordinate;
varying vec2 maskCoordinate;

void main()
{
    gl_Position = vec4(position,1.0);
    textureCoordinate = position.xy*0.5+0.5;
    maskCoordinate = inputTextureCoordinate.xy;
}