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
varying vec2 textureCoordinate2;

uniform sampler2D inputImageTexture;
uniform sampler2D inputImageTexture2;
uniform float alpha;
uniform int blendtype;

float blendSoftLight(float base, float blend)
{
    float color1 = 2.0 * base * blend + base * base * (1.0 - 2.0 * blend);
    float color2 = sqrt(base) * (2.0 * blend - 1.0) + 2.0 * base * (1.0 - blend);
    return mix(color1, color2, step(0.5, blend));
}

void main()
{
    vec4 dstColor = texture(inputImageTexture,textureCoordinate);
    vec4 srcColor = texture(inputImageTexture2,textureCoordinate2);
    float alpha2 = dstColor.a*alpha;
    vec4 blendColor;
    if(blendtype == 0)
    {
        blendColor = vec4(dstColor.rgb, alpha2);
    }
    else if(blendtype == 1)
    {
        blendColor = srcColor * dstColor;
        blendColor.a = alpha2;
    }
    else if(blendtype == 2)
    {
        blendColor = vec4(blendSoftLight(srcColor.r, dstColor.r),blendSoftLight(srcColor.g, dstColor.g),blendSoftLight(srcColor.b, dstColor.b),alpha2);
    }
    gl_FragColor =  mix(srcColor,blendColor,alpha2);

   // gl_FragColor = vec4(1.0,0.0,0.0,1.0);
    gl_FragColor.a=1.0;
}
