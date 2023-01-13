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
out vec4 outColor;
uniform sampler2D inputImageTexture;
uniform sampler2D inputImageTexture2;
uniform float alpha;



void main()
{
    vec4 color = texture(inputImageTexture,textureCoordinate)*alpha / 5.0;
    outColor =color;
}