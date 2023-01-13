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
uniform sampler2D inputImageTexture;
in  vec2 textureCoordinate;
in  vec4 textureShift_1;
in  vec4 textureShift_2;
in  vec4 textureShift_3;
in  vec4 textureShift_4;

out vec4 out_FragColor;

void main()
{
    vec4 iColor = texture(inputImageTexture, textureCoordinate);
    vec4 sum = iColor;
    sum += texture(inputImageTexture, (textureCoordinate + textureShift_1.xy) * 0.5);
    sum += texture(inputImageTexture, (textureCoordinate + textureShift_1.zw) * 0.5);
    sum += texture(inputImageTexture, textureShift_1.xy);
    sum += texture(inputImageTexture, textureShift_1.zw);
    sum += texture(inputImageTexture, (textureShift_1.xy + textureShift_2.xy) * 0.5);
    sum += texture(inputImageTexture, (textureShift_1.zw + textureShift_2.zw) * 0.5);
    sum += texture(inputImageTexture, textureShift_2.xy);
    sum += texture(inputImageTexture, textureShift_2.zw);
    sum += texture(inputImageTexture, (textureShift_2.xy + textureShift_3.xy) * 0.5);
    sum += texture(inputImageTexture, (textureShift_2.zw + textureShift_3.zw) * 0.5);
    sum += texture(inputImageTexture, textureShift_3.xy);
    sum += texture(inputImageTexture, textureShift_3.zw);
    sum += texture(inputImageTexture, (textureShift_3.xy + textureShift_4.xy) * 0.5);
    sum += texture(inputImageTexture, (textureShift_3.zw + textureShift_4.zw) * 0.5);
    sum += texture(inputImageTexture, textureShift_4.xy);
    sum += texture(inputImageTexture, textureShift_4.zw);
    sum *= 0.05882;
    out_FragColor = sum;
}