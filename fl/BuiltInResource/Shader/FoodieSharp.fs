#ifdef GL_ES
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

uniform sampler2D inputImageTexture;
uniform sampler2D inputImageTexture2;
varying  vec2 textureCoordinate;
uniform float alpha;
void main()
{
    vec4 iColor = texture2D(inputImageTexture, textureCoordinate);
    vec4 meanColor = texture2D(inputImageTexture2, textureCoordinate);
    gl_FragColor = vec4(iColor.rgb * alpha + meanColor.rgb * (1.0 - alpha), iColor.a);
}