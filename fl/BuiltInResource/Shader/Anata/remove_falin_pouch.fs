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

varying  vec2 textureCoordinate;
varying  vec2 maskCoordinate;

uniform sampler2D inputImageTexture;
uniform sampler2D inputImageTexture2;
uniform sampler2D inputImageTexture3;
uniform sampler2D inputMaskTexture;

uniform  float falinAlpha;
uniform  float pouchAlpha;
uniform  float BrightEyeAlpha;
void main()
{
    vec4 color = texture2D(inputImageTexture, textureCoordinate);
    vec4 maskColor = texture2D(inputMaskTexture, maskCoordinate);

    if(maskColor.r>0.01&&maskColor.b<0.1)
    {
        vec3 resultColor = color.rgb;
        vec4 sum = texture2D(inputImageTexture2, textureCoordinate);
        vec4 sum2 = texture2D(inputImageTexture3, textureCoordinate);
        vec3 diff1 = clamp((mix(color.rgb, sum2.rgb, sum.a) - sum.rgb) * 1.3 + 0.03 * sum2.rgb, 0.0, 0.2);
        vec3 diff2 = clamp((mix(color.rgb, sum2.rgb, sum.a) - color.rgb) * 1.3 + 0.03 * sum2.rgb, 0.0, 0.2);
        vec3 diffSign = sign(diff1);
        diff1 = diff1 * diffSign;
        diff2 = diff2 * diffSign;
        diff1 = min(diff1, diff2);
        diff1 = max(diff1, diff1 * 0.5);
        diff1 = diff1 * diffSign;

        diff1 = clamp(color.rgb + diff1, 0.0, 1.0);

        color.rgb = mix(color.rgb, diff1, pouchAlpha * maskColor.r * sum.a);
        resultColor = color.rgb;
        gl_FragColor = vec4(resultColor, 1.0);
        return;
    }

    if(maskColor.b>0.01&&maskColor.r<0.1)//bright eye
    {
        vec2 step1 = vec2(0.00208, 0.0);
        vec2 step2 = vec2(0.0, 0.00134);
        vec3 sumColor = vec3(0.0, 0.0, 0.0);
        for(float t = -2.0; t < 2.5; t += 1.0)
        {
            for(float p = -2.0;p < 2.5; p += 1.0)
            {
                sumColor += texture2D(inputImageTexture,textureCoordinate+ t * step1 + p * step2).rgb;
            }
        }
        sumColor = sumColor * 0.04;
        sumColor = clamp(sumColor + (color.rgb - sumColor) * 3.0, 0.0, 1.0);
        sumColor = max(color.rgb, sumColor);
        gl_FragColor = vec4(mix(color.rgb, sumColor, maskColor.b*BrightEyeAlpha*0.8),color.a);
        return;

    }

    if(maskColor.g>0.01)
    {
        vec4 sum = texture2D(inputImageTexture2, textureCoordinate);
        vec4 sum2 = texture2D(inputImageTexture3, textureCoordinate);
        vec3 imDiff = clamp((mix(color.rgb, sum2.rgb, sum.a) - sum.rgb) * 1.4 + 0.05 * sum2.rgb, 0.0, 0.3);
        imDiff = min(color.rgb + imDiff, 1.0);
        gl_FragColor =vec4(mix(color.rgb,imDiff,falinAlpha * maskColor.g * sum.a),color.a);
        return;
    }   
}