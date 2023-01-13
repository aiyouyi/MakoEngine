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

uniform sampler2D inputImageTexture;

void main()
{
    vec4 color = texture2D(inputImageTexture,textureCoordinate);
    gl_FragColor = color;
}
