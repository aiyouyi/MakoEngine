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

uniform sampler2D srcImage;
uniform sampler2D MaskImageTexture;
uniform sampler2D weight1;
uniform sampler2D weight2;
uniform sampler2D LUTTexture;
varying  vec2 textureCoordinate;
varying  vec2 textureCoordinate2;

uniform vec4 mouthColor;
uniform vec4 metalColor;
uniform float factorScale;
uniform float factorShift;
uniform float factorShrink;
uniform float factorThreshold;
uniform float gloss_alpha;
uniform float metallight_alpha;

uniform int  HasLips;
uniform int  HasTeeth;

uniform float TeethAlpha;

float BlendOverlayf(float base, float blend)
{
    return (base<0.5?(2.0*(base)*(blend)):(1.0-2.0*(1.0-(base))*(1.0-(blend))));
}

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
    vec3 source=texture2D(srcImage,textureCoordinate2).rgb;
    vec4 lipstickMasks=texture2D(MaskImageTexture,textureCoordinate);
    vec3 target_color=source;
    if(HasLips > 0)
    {
        if (lipstickMasks.r>0.0)
        {
            float alpha=lipstickMasks.r*mouthColor.a;
            float contrast_mask=lipstickMasks.r*metallight_alpha;
            float gray=0.299*source.r+0.587*source.g+0.114*source.b;
            float contrast=max(min((factorScale+factorShift)*contrast_mask+0.5*(1.0-contrast_mask), 1.0),0.0);
            if(contrast<0.5)
            {
                contrast=0.5-(0.5-contrast)*factorShrink;
            }

            float blend_weight=texture2D(weight1,vec2(gray,0)).r;
            float level_weight=texture2D(weight2,vec2(gray,0)).r*alpha*2.0;
            vec3 color=mix(mouthColor.rgb*source,mouthColor.rgb,blend_weight);
            target_color=mix(source,color,level_weight);
            float diff=max(gray-factorThreshold,0.0)*alpha;
            target_color=vec3(1.0)-(vec3(1.0)-target_color)*vec3(1.0-diff);

            vec3 res_color = target_color;
            res_color.r=BlendOverlayf(target_color.r,contrast);
            res_color.g=BlendOverlayf(target_color.g,contrast);
            res_color.b=BlendOverlayf(target_color.b,contrast);
            target_color =mix(target_color, res_color, alpha);

            vec3 HSL = RGBtoHSL(target_color.bgr);
            vec3 HSL2 = RGBtoHSL(source.bgr);
            HSL.b = HSL.b*0.2+HSL2.b*0.8;
            target_color.bgr = HSLtoRGB(HSL);
            target_color = mix(source, target_color, lipstickMasks.r);
        }
    }
    if(HasTeeth>0)
    {
        float blueColor = target_color.b * 15.0;
        vec2 quad1;
        quad1.y = floor(floor(blueColor) / 4.0);
        quad1.x = floor(blueColor) - (quad1.y * 4.0);
        vec2 quad2;
        quad2.y = floor(ceil(blueColor) / 4.0);
        quad2.x = ceil(blueColor) - (quad2.y * 4.0);
        vec2 texPos1;
        texPos1.x = (quad1.x * 0.25) + 0.5/64.0 + ((0.25 - 1.0/64.0) * target_color.r);
        texPos1.y = (quad1.y * 0.25) + 0.5/64.0 + ((0.25 - 1.0/64.0) * target_color.g);
        vec2 texPos2;
        texPos2.x = (quad2.x * 0.25) + 0.5/64.0 + ((0.25 - 1.0/64.0) * target_color.r);
        texPos2.y = (quad2.y * 0.25) + 0.5/64.0 + ((0.25 - 1.0/64.0) * target_color.g);

        vec4 newColor1 = texture2D(LUTTexture, texPos1);
        vec4 newColor2 = texture2D(LUTTexture, texPos2);
        vec4 newColor = mix(newColor1, newColor2, fract(blueColor));
        target_color = mix(target_color,newColor.rgb, lipstickMasks.g*TeethAlpha);
     //   target_color = lipstickMasks.ggg;
    }
    gl_FragColor= vec4(target_color,1.0);

}