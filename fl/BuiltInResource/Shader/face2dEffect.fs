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

uniform sampler2D inputImageTexture;
uniform sampler2D inputImageTexture2;//offset

void main()
{
    vec4 maskColor = texture2D(inputImageTexture2, maskCoordOut);
    float alpha = maskColor.r;
    vec4 color = texture2D(inputImageTexture,textureCoordinate);
    color.a *= alpha;
    gl_FragColor = color;
}