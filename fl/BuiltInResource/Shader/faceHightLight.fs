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
uniform sampler2D inputImageTexture2;
uniform sampler2D shNormalTexture;
uniform sampler2D hightLightLookupTexture;
uniform sampler2D shadowLookupTexture;
uniform lowp float alpha;
uniform lowp vec3 cameraZRotation;

void main()
{
    float shMask = texture2D(inputImageTexture2, maskCoordOut).r - 0.5;
    vec3 color = texture2D(inputImageTexture,textureCoordinate).rgb;
    vec3 shNormal = texture2D(shNormalTexture, maskCoordOut).rgb * 2.0 - 1.0;

    vec3 resShadow;
    vec3 resHighlight;
    float rate;
    rate = clamp(dot(cameraZRotation, shNormal),0.0,1.0);
    rate = rate * rate * alpha * 1.5;
         
    resShadow.r = texture2D(shadowLookupTexture, vec2(color.r, color.r)).r;
    resShadow.g = texture2D(shadowLookupTexture, vec2(color.g, color.g)).r;
    resShadow.b = texture2D(shadowLookupTexture, vec2(color.b, color.b)).r;
         
    resHighlight.r = texture2D(hightLightLookupTexture, vec2(color.r, color.r)).r;
    resHighlight.g = texture2D(hightLightLookupTexture, vec2(color.g, color.g)).r;
    resHighlight.b = texture2D(hightLightLookupTexture, vec2(color.b, color.b)).r;
         
    resHighlight = mix(color, resHighlight, max(shMask,0.0));
    resHighlight = mix(resHighlight, resShadow, max(-shMask,0.0));
    resHighlight = mix(color, resHighlight, rate);       
    gl_FragColor = vec4(resHighlight, 1.0);
}