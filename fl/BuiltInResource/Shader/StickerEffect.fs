#ifdef GL_ES//for discriminate GLES & GL
#ifdef GL_FRAGMENT_PRECISION_HIGH
precision highp float;
#else
precision mediump float;
#endif
#else
#define highp
#define mediump
#define lowp
#endif
varying highp vec2 v_textureCoordinate;
uniform sampler2D u_inputImageTexture;
uniform vec4 u_color;

void main()
{
	gl_FragColor = texture2D(u_inputImageTexture, v_textureCoordinate)*u_color;
}