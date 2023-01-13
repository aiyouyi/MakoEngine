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
varying vec2 textureCoordinate;
varying vec2 maskCoordOut;
varying vec2 FaceScaleRadiusOut;

uniform sampler2D inputImageTexture;
uniform sampler2D inputImageTexture2;//offset

uniform mat2 transformMatrix;

void main()
{
    vec4 maskColor = texture2D(inputImageTexture2, maskCoordOut);
	vec2 newCoords = vec2(0.0,0.0);
    vec2 xy = maskColor.rg;//vec2(x,y);
    newCoords = (xy - 0.498) * 0.25;

	newCoords.x*=FaceScaleRadiusOut.x;
	newCoords.y*=FaceScaleRadiusOut.y;
    newCoords *= transformMatrix;
    gl_FragColor = texture2D(inputImageTexture,textureCoordinate + newCoords);
}
