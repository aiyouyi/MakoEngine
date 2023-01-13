attribute vec3 a_position;
attribute vec2 a_texcoord;
attribute vec2 a_texcoordMask;
varying vec2 textureCoordinate;
varying vec2 maskCoordOut;
void main()
{
	gl_Position = vec4(a_position,1.0);
	textureCoordinate = a_texcoord;
	maskCoordOut = a_texcoordMask;
}