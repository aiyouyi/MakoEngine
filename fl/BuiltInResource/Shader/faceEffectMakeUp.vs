attribute vec2 a_position;
attribute vec2 a_texcoord;
varying vec2 textureCoordinate;


void main()
{
        gl_Position = vec4(a_position,0.5,1.0);
        textureCoordinate = a_texcoord;
}
