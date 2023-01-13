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
uniform vec4 HSVColors;

//色调转换
 vec3 RGBtoHCV(vec3 rgb){
    vec4 p,q;
    if(rgb.g<rgb.b)
    {
        p = vec4(rgb.bg,-1.0,0.66666667);
    }
    else
    {
        p = vec4(rgb.gb,0.0,-0.33333333);
    }

    if(rgb.g<rgb.b)
    {
        q = vec4(p.xyw,rgb.r);
    }
    else
    {
        q = vec4(rgb.r,p.yzx);
    }

    float c=q.x-min(q.w,q.y);
    float h=abs((q.w-q.y)/(6.0*c+0.001)+q.z);
    return vec3(h,c,q.x);
 }

 vec3 RGBtoHSL(vec3 rgb){
    vec3 hcv=RGBtoHCV(rgb);
    float l=hcv.z-hcv.y*0.5;
    float s=hcv.y/(1.0-abs(l*2.0-1.0)+0.001);
    return vec3(hcv.x,s,l);
 }

 vec3 HSLtoRGB(vec3 hsl){
    vec3 rgb;
    float x=hsl.x*6.0;
    rgb.r=abs(x-3.0)-1.0;
    rgb.g=2.0-abs(x-2.0);
    rgb.b=2.0-abs(x-4.0);
    rgb=clamp(rgb,0.0,1.0);
    float c=(1.0-abs(2.0*hsl.z-1.0))*hsl.y;
    rgb=clamp((rgb-0.5)*c+hsl.z,0.0,1.0);
    return rgb;
 }


void main()
{
    vec4 srcColor = texture(inputImageTexture,textureCoordinate);
    vec3 HSL = RGBtoHSL(srcColor.bgr);
    vec3 HSL2 = RGBtoHSL(HSVColors.bgr);
    HSL.r= HSL2.r;
    srcColor.bgr = mix(srcColor.bgr,HSLtoRGB(HSL),HSVColors.a);
    
    outColor = srcColor;
}