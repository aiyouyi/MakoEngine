#version 300 es
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
in vec2 textureCoordinate;

uniform sampler2D inputImageTexture;
uniform  vec2 stepSize;
out vec4 out_FragColor;

void main()
{
    vec4 iColor = texture(inputImageTexture, textureCoordinate);
    vec4 sum = iColor * 0.5;
    sum += texture(inputImageTexture, textureCoordinate + vec2(0.5, 1.0) * stepSize);
    sum += texture(inputImageTexture, textureCoordinate + vec2(1.0, -0.5) * stepSize);
    sum += texture(inputImageTexture, textureCoordinate + vec2(-0.5, -1.0) * stepSize);
    sum += texture(inputImageTexture, textureCoordinate + vec2(-1.0, 0.5) * stepSize);
    out_FragColor = vec4(sum * 0.22222);
}