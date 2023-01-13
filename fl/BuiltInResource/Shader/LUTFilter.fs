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
uniform sampler2D inputImageTextureAdjust;
uniform sampler2D inputImageTextureClear;
uniform sampler2D inputImageTextureType;
varying  vec2 textureCoordinate;
uniform float alpha;
uniform float alphaAdjust;
uniform float alphaClear;
uniform float alphaType;

void main() {

    vec4 textureColor = texture2D(inputImageTexture, textureCoordinate);
    if(alphaAdjust>0.01)
    {
        float blueColor = textureColor.b * 63.0;
        vec2 quad1;
        quad1.y = floor(floor(blueColor) / 8.0);
        quad1.x = floor(blueColor) - (quad1.y * 8.0);
        vec2 quad2;
        quad2.y = floor(ceil(blueColor) / 8.0);
        quad2.x = ceil(blueColor) - (quad2.y * 8.0);
        vec2 texPos1;
        texPos1.x = (quad1.x * 0.125) + 0.5/512.0 + ((0.125 - 1.0/512.0) * textureColor.r);
        texPos1.y = (quad1.y * 0.125) + 0.5/512.0 + ((0.125 - 1.0/512.0) * textureColor.g);
        vec2 texPos2;
        texPos2.x = (quad2.x * 0.125) + 0.5/512.0 + ((0.125 - 1.0/512.0) * textureColor.r);
        texPos2.y = (quad2.y * 0.125) + 0.5/512.0 + ((0.125 - 1.0/512.0) * textureColor.g);
        vec4 newColor1 = texture2D(inputImageTextureAdjust, texPos1);
        vec4 newColor2 = texture2D(inputImageTextureAdjust, texPos2);
        vec4 newColor = mix(newColor1, newColor2, fract(blueColor));
        textureColor = mix(textureColor, vec4(newColor.rgb, textureColor.w), alphaAdjust);
    }
    if(alpha>0.01)
    {
        float blueColor = textureColor.b * 63.0;
        vec2 quad1;
        quad1.y = floor(floor(blueColor) / 8.0);
        quad1.x = floor(blueColor) - (quad1.y * 8.0);
        vec2 quad2;
        quad2.y = floor(ceil(blueColor) / 8.0);
        quad2.x = ceil(blueColor) - (quad2.y * 8.0);
        vec2 texPos1;
        texPos1.x = (quad1.x * 0.125) + 0.5/512.0 + ((0.125 - 1.0/512.0) * textureColor.r);
        texPos1.y = (quad1.y * 0.125) + 0.5/512.0 + ((0.125 - 1.0/512.0) * textureColor.g);
        vec2 texPos2;
        texPos2.x = (quad2.x * 0.125) + 0.5/512.0 + ((0.125 - 1.0/512.0) * textureColor.r);
        texPos2.y = (quad2.y * 0.125) + 0.5/512.0 + ((0.125 - 1.0/512.0) * textureColor.g);
        vec4 newColor1 = texture2D(inputImageTexture2, texPos1);
        vec4 newColor2 = texture2D(inputImageTexture2, texPos2);
        vec4 newColor = mix(newColor1, newColor2, fract(blueColor));
        textureColor = mix(textureColor, vec4(newColor.rgb, textureColor.w), alpha);
    }

    if(alphaClear>0.01)
    {
        float blueColor = textureColor.b * 63.0;
        vec2 quad1;
        quad1.y = floor(floor(blueColor) / 8.0);
        quad1.x = floor(blueColor) - (quad1.y * 8.0);
        vec2 quad2;
        quad2.y = floor(ceil(blueColor) / 8.0);
        quad2.x = ceil(blueColor) - (quad2.y * 8.0);
        vec2 texPos1;
        texPos1.x = (quad1.x * 0.125) + 0.5/512.0 + ((0.125 - 1.0/512.0) * textureColor.r);
        texPos1.y = (quad1.y * 0.125) + 0.5/512.0 + ((0.125 - 1.0/512.0) * textureColor.g);
        vec2 texPos2;
        texPos2.x = (quad2.x * 0.125) + 0.5/512.0 + ((0.125 - 1.0/512.0) * textureColor.r);
        texPos2.y = (quad2.y * 0.125) + 0.5/512.0 + ((0.125 - 1.0/512.0) * textureColor.g);
        vec4 newColor1 = texture2D(inputImageTextureClear, texPos1);
        vec4 newColor2 = texture2D(inputImageTextureClear, texPos2);
        vec4 newColor = mix(newColor1, newColor2, fract(blueColor));
        textureColor = mix(textureColor, vec4(newColor.rgb, textureColor.w), alphaClear);
    }
    if(alphaType>0.01)
    {
        float blueColor = textureColor.b * 63.0;
        vec2 quad1;
        quad1.y = floor(floor(blueColor) / 8.0);
        quad1.x = floor(blueColor) - (quad1.y * 8.0);
        vec2 quad2;
        quad2.y = floor(ceil(blueColor) / 8.0);
        quad2.x = ceil(blueColor) - (quad2.y * 8.0);
        vec2 texPos1;
        texPos1.x = (quad1.x * 0.125) + 0.5/512.0 + ((0.125 - 1.0/512.0) * textureColor.r);
        texPos1.y = (quad1.y * 0.125) + 0.5/512.0 + ((0.125 - 1.0/512.0) * textureColor.g);
        vec2 texPos2;
        texPos2.x = (quad2.x * 0.125) + 0.5/512.0 + ((0.125 - 1.0/512.0) * textureColor.r);
        texPos2.y = (quad2.y * 0.125) + 0.5/512.0 + ((0.125 - 1.0/512.0) * textureColor.g);
        vec4 newColor1 = texture2D(inputImageTextureType, texPos1);
        vec4 newColor2 = texture2D(inputImageTextureType, texPos2);
        vec4 newColor = mix(newColor1, newColor2, fract(blueColor));
        textureColor = mix(textureColor, vec4(newColor.rgb, textureColor.w), alphaType);
    }
    gl_FragColor = vec4(textureColor.rgb,1.0);
}