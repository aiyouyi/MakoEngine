attribute vec2 a_position;
attribute vec2 a_texcoordMask;
attribute vec2 a_FaceScaleRadius;
varying vec2 textureCoordinate;
varying vec2 maskCoordOut;
varying vec2 FaceScaleRadiusOut;
void main()
{
	gl_Position = vec4(a_position*2.0-1.0,0.5,1.0);
	textureCoordinate = a_position;
	maskCoordOut = a_texcoordMask;
	FaceScaleRadiusOut = a_FaceScaleRadius;
}