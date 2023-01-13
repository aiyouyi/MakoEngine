attribute vec2 a_position;
attribute vec2 a_texcoord;
varying vec2 textureCoordinate;
varying vec2 textureCoordinate2;


void main()
{
        gl_Position = vec4(a_position*2.0-1.0,0.5,1.0);
        textureCoordinate2 = a_position.xy;
        textureCoordinate = a_texcoord;
}
