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

//柔光
float blendSoftLight(float base, float blend)
{
    float color1 = 2.0 * base * blend + base * base * (1.0 - 2.0 * blend);
    float color2 = sqrt(base) * (2.0 * blend - 1.0) + 2.0 * base * (1.0 - blend);
    return mix(color1, color2, step(0.5, blend));
}
//滤色
float BlendScreenf(float base, float blend) {
    return (1.0 - ((1.0 - (base)) * (1.0 - (blend))));
}
//变暗
float BlendDarkenf(float base, float blend) {
    return min(blend, base);
}
//颜色加深
float BlendColorBurnf(float base, float blend) {
    return ((blend == 0.0) ? blend : max((1.0 - ((1.0 - (base)) / (blend))), 0.0));
}
//线性加深
float BlendLinearBurnf(float base, float blend) {
    return max(base + blend - 1.0, 0.0);
}
//变亮
float BlendLightenf(float base, float blend) {
    return max(blend, base);
}
//颜色减淡
float BlendColorDodgef(float base, float blend) {
    return ((blend == 1.0) ? blend : min((base) / (1.0 - (blend)), 1.0));
}
//线性减淡
float BlendLinearDodgef(float base, float blend) {
    return min(base + blend, 1.0);
}
//深色浅色
vec4 BlendLighterDarkerColor(vec4 base, vec4 blend,int flag)
{
    vec4 a = step(blend, base);
    vec4 color;
    if(flag == 0)
    {
        color = a*blend + (1.0-a)*base;
        return color;
    }
    else
    {
        color = a*base + (1.0-a)*blend;
        return color;
    }
}
//叠加
float BlendOverlayf(float base, float blend) {
    return (base < 0.5 ? (2.0 * (base) * (blend)) : (1.0 - 2.0 * (1.0 - (base)) * (1.0 - (blend))));
}

void main()
{
    vec4 dstColor = texture2D(inputImageTexture,textureCoordinate);
    vec4 srcColor = texture2D(inputImageTexture2,textureCoordinate2);
    // dstColor.a = dot(srcColor.rgb, vec3(0.2989, 0.5870, 0.1140));

	float alpha2 = dstColor.a * alpha;
    vec4 blendColor;

    if(blendtype == 0)
    {
        blendColor = vec4(dstColor.rgb,alpha2);
    }
    else if(blendtype == 1)
    {
        blendColor = srcColor * dstColor;
        blendColor.a =alpha2;
    }
    else if(blendtype == 2)
    {
        blendColor = vec4(blendSoftLight(srcColor.r, dstColor.r),blendSoftLight(srcColor.g, dstColor.g),blendSoftLight(srcColor.b, dstColor.b),alpha2);
        //blendColor = vec4(dstColor.rgb, 1.0);
    }
    else if(blendtype == 3)
    {
        blendColor = vec4(BlendScreenf(srcColor.r, dstColor.r),BlendScreenf(srcColor.g, dstColor.g),BlendScreenf(srcColor.b, dstColor.b),alpha2);
    }
        else if(blendtype == 4)
    {
        blendColor = vec4(BlendDarkenf(srcColor.r, dstColor.r),BlendDarkenf(srcColor.g, dstColor.g),BlendDarkenf(srcColor.b, dstColor.b),alpha2);
    }
    else if(blendtype == 5)
    {
        blendColor = vec4(BlendColorBurnf(srcColor.r, dstColor.r),BlendColorBurnf(srcColor.g, dstColor.g),BlendColorBurnf(srcColor.b, dstColor.b),alpha2);
    }
    else if(blendtype == 6)
    {
        blendColor = vec4(BlendLinearBurnf(srcColor.r, dstColor.r),BlendLinearBurnf(srcColor.g, dstColor.g),BlendLinearBurnf(srcColor.b, dstColor.b),alpha2);
    }
    else if(blendtype == 7)
    {
        blendColor = vec4(BlendLighterDarkerColor(srcColor, dstColor, 0).rgb,alpha2);
    }
    else if(blendtype == 8)
    {
        blendColor = vec4(BlendLightenf(srcColor.r, dstColor.r),BlendLightenf(srcColor.g, dstColor.g),BlendLightenf(srcColor.b, dstColor.b),alpha2);
    }
    else if(blendtype == 9)
    {
        blendColor = vec4(BlendColorDodgef(srcColor.r, dstColor.r),BlendColorDodgef(srcColor.g, dstColor.g),BlendColorDodgef(srcColor.b, dstColor.b),alpha2);
    }
    else if(blendtype == 10)
    {
        blendColor = vec4(BlendLinearDodgef(srcColor.r, dstColor.r),BlendLinearDodgef(srcColor.g, dstColor.g),BlendLinearDodgef(srcColor.b, dstColor.b),alpha2);
    }
    else if(blendtype == 11)
    {
        blendColor = vec4(BlendLighterDarkerColor(srcColor, dstColor, 1).rgb,alpha2);
    }
    else if(blendtype == 12)
    {
        blendColor = vec4(BlendOverlayf(srcColor.r, dstColor.r),BlendOverlayf(srcColor.g, dstColor.g),BlendOverlayf(srcColor.b, dstColor.b),alpha2);
    }
    else
    {
        blendColor = vec4(dstColor.rgb,alpha2);
    }
    
    gl_FragColor = mix(srcColor,blendColor,blendColor.a);
    gl_FragColor.a = 1.0;

}