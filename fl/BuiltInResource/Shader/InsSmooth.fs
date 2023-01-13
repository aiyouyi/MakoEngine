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
uniform sampler2D inputImageTexture2;
uniform sampler2D inputImageTextureSkin;
uniform sampler2D inputImageTextureFace;
varying  vec2 textureCoordinate;
uniform float alpha;

void main()
{
    vec4 iColor = texture2D(inputImageTexture, textureCoordinate);
    vec4 meanColor = texture2D(inputImageTexture2, textureCoordinate);
    float skin = texture2D(inputImageTextureSkin, textureCoordinate).r;
   // float face = texture2D(inputImageTextureFace, textureCoordinate).r;
    vec3 smoothColor = meanColor.a * iColor.rgb + meanColor.rgb;
   // gl_FragColor = vec4(mix(iColor.rgb, smoothColor, skin*faceAlpha), 1.0);
    gl_FragColor = vec4(mix(iColor.rgb, smoothColor, skin*alpha*0.8), 1.0);
    //float smoothAlpha = (1.0-meanColor.a);//*skin*face*alpha;//*sqrt(oriMask);
    //gl_FragColor = vec4(smoothAlpha,smoothAlpha,smoothAlpha,1.0);
}
