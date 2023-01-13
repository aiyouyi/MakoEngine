attribute vec3 a_position;
attribute vec2 a_texcoord;
varying vec2 textureCoordinate;
varying vec2 maskCoordOut;
void main()
{
	gl_Position = vec4(a_position,1.0);
	textureCoordinate = a_position.xy*0.5+0.5;
	maskCoordOut = a_texcoord;
}