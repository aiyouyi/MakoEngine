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
varying  vec2 textureCoordinate;
varying  vec4 textureShift_1;
varying  vec4 textureShift_2;
varying  vec4 textureShift_3;
varying  vec4 textureShift_4;

void main()
{
    vec4 iColor = texture2D(inputImageTexture, textureCoordinate);

    vec4 tmpColor = iColor;
    vec4 sum = vec4(tmpColor.rgb * tmpColor.a, tmpColor.a);

    tmpColor = texture2D(inputImageTexture, (textureCoordinate + textureShift_1.xy) * 0.5);
    sum += vec4(tmpColor.rgb * tmpColor.a, tmpColor.a);
    tmpColor = texture2D(inputImageTexture, (textureCoordinate + textureShift_1.zw) * 0.5);
    sum += vec4(tmpColor.rgb * tmpColor.a, tmpColor.a);
    tmpColor = texture2D(inputImageTexture, textureShift_1.xy);
    sum += vec4(tmpColor.rgb * tmpColor.a, tmpColor.a);
    tmpColor = texture2D(inputImageTexture, textureShift_1.zw);
    sum += vec4(tmpColor.rgb * tmpColor.a, tmpColor.a);

    tmpColor = texture2D(inputImageTexture, (textureShift_1.xy + textureShift_2.xy) * 0.5);
    sum += vec4(tmpColor.rgb * tmpColor.a, tmpColor.a);
    tmpColor = texture2D(inputImageTexture, (textureShift_1.zw + textureShift_2.zw) * 0.5);
    sum += vec4(tmpColor.rgb * tmpColor.a, tmpColor.a);
    tmpColor = texture2D(inputImageTexture, textureShift_2.xy);
    sum += vec4(tmpColor.rgb * tmpColor.a, tmpColor.a);
    tmpColor = texture2D(inputImageTexture, textureShift_2.zw);
    sum += vec4(tmpColor.rgb * tmpColor.a, tmpColor.a);

    tmpColor = texture2D(inputImageTexture, (textureShift_2.xy + textureShift_3.xy) * 0.5);
    sum += vec4(tmpColor.rgb * tmpColor.a, tmpColor.a);
    tmpColor = texture2D(inputImageTexture, (textureShift_2.zw + textureShift_3.zw) * 0.5);
    sum += vec4(tmpColor.rgb * tmpColor.a, tmpColor.a);
    tmpColor = texture2D(inputImageTexture, textureShift_3.xy);
    sum += vec4(tmpColor.rgb * tmpColor.a, tmpColor.a);
    tmpColor = texture2D(inputImageTexture, textureShift_3.zw);
    sum += vec4(tmpColor.rgb * tmpColor.a, tmpColor.a);

    tmpColor = texture2D(inputImageTexture, (textureShift_3.xy + textureShift_4.xy) * 0.5);
    sum += vec4(tmpColor.rgb * tmpColor.a, tmpColor.a);
    tmpColor = texture2D(inputImageTexture, (textureShift_3.zw + textureShift_4.zw) * 0.5);
    sum += vec4(tmpColor.rgb * tmpColor.a, tmpColor.a);
    tmpColor = texture2D(inputImageTexture, textureShift_4.xy);
    sum += vec4(tmpColor.rgb * tmpColor.a, tmpColor.a);
    tmpColor = texture2D(inputImageTexture, textureShift_4.zw);
    sum += vec4(tmpColor.rgb * tmpColor.a, tmpColor.a);

    if (iColor.a > 0.01)
    {
        sum.rgb /= sum.a;
        sum.a *= 0.05882;
    }
    else
    {
        sum = iColor;
    }

    gl_FragColor = sum;
}