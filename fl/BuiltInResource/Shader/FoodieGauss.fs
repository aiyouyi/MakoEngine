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
varying  vec2 blurCoordinates[5];
void main()
{
    vec4 sum = vec4(0.0);
    sum += texture2D(inputImageTexture, blurCoordinates[0]) * 0.398943;
    sum += texture2D(inputImageTexture, blurCoordinates[1]) * 0.295963;
    sum += texture2D(inputImageTexture, blurCoordinates[2]) * 0.295963;
    sum += texture2D(inputImageTexture, blurCoordinates[3]) * 0.004566;
    sum += texture2D(inputImageTexture, blurCoordinates[4]) * 0.004566;
    gl_FragColor = sum;
}