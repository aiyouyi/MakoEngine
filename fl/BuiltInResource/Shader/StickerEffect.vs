attribute vec3 a_position;
attribute vec2 a_inputTextureCoordinate;
varying vec2 v_textureCoordinate;
uniform mat4 u_matWVP;
uniform vec4 faceoriRect;
void main()
{
	vec4 pos = u_matWVP*vec4(a_position,1.0);
	pos /= pos.w;
	pos.xy = (pos.xy + vec2(1.0)) * faceoriRect.zw*0.5 + faceoriRect.xy;
	pos.xy = pos.xy *2.0 -vec2(1.0);
	gl_Position = pos;
	v_textureCoordinate = vec2(a_inputTextureCoordinate.x, 1.0 - a_inputTextureCoordinate.y);
}
