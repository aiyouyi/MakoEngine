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
varying  vec2 textureCoordinate;
varying  vec4 textureShift_1;
varying  vec4 textureShift_2;
varying  vec4 textureShift_3;
varying  vec4 textureShift_4;
void main()
{
    float alpha = 0.6;
    vec4 oriColor = texture2D(inputImageTexture, textureCoordinate);
    float sumMask = 0.28125*2.0;
    vec4 sum = oriColor * sumMask;
    
    vec4 iColor = texture2D(inputImageTexture, textureShift_1.xy);
    float mask = 0.21875*alpha;
    sum += iColor * mask;
    sumMask += mask;
    
    iColor = texture2D(inputImageTexture, textureShift_1.zw);
    sum += iColor * mask;
    sumMask += mask;

    iColor = texture2D(inputImageTexture, (textureShift_1.xy+textureCoordinate)*0.5);
    mask = 0.21875*alpha*1.5;
    sum += iColor * mask;
    sumMask += mask;
    
    iColor = texture2D(inputImageTexture, (textureShift_1.zw+textureCoordinate)*0.5);
    sum += iColor * mask;
    sumMask += mask;
    

    iColor = texture2D(inputImageTexture, (textureShift_1.xy+textureShift_2.xy)*0.5);
    mask = 0.21875*alpha*0.9;
    sum += iColor * mask;
    sumMask += mask;
    
    iColor = texture2D(inputImageTexture, (textureShift_1.zw+textureShift_2.zw)*0.5);
    sum += iColor * mask;
    sumMask += mask;

    
    iColor = texture2D(inputImageTexture, textureShift_2.xy);
    mask = 0.109375*alpha*0.9;
    sum += iColor * mask;
    sumMask += mask;

    iColor = texture2D(inputImageTexture, textureShift_2.zw);
    sum += iColor * mask;
    sumMask += mask;
    


    
    iColor = texture2D(inputImageTexture, textureShift_3.xy);
    mask = 0.03125*alpha*0.8;
    sum += iColor * mask;
    sumMask += mask;
    
    iColor = texture2D(inputImageTexture, textureShift_3.zw);
    sum += iColor * mask;
    sumMask += mask;
    
    sum  = sum / sumMask;
    gl_FragColor = mix(oriColor, sum, 1.0 - oriColor.a);
}