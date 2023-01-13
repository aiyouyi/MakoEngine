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

     
vec3 rgb2hsv(vec3 c)
{
    vec4 K = vec4(0.0, - 0.33333, 0.66667, -1.0);
    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));
    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    float s = 0.0; //for optimization, set0.0 right result: d / (q.x + e)
    vec3 hsv = vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)),s , q.x);
    return hsv;
}


void main()
{
    vec3 srcColor = texture2D(inputImageTexture, textureCoordinate).rgb;
    vec3 hsvSpace = rgb2hsv(srcColor.rgb);   //[hue, saturation, value]
    float hue = hsvSpace.x;
    float value = hsvSpace.z;
    float opacity = 1.0;
    //float white = srcColor.r + srcColor.g + srcColor.b;
    // float lowH = 0.8;
    // float highH = 0.8;
    // //skin condition: the range of hue:[0.16,0.18] or [0.89,0.91],the value can not be too low
    // if ((lowH <= hue && hue <= highH) || value <= 0.3)
    // {
    //     opacity = 0.0;
    // }
    // if (lowH-0.02 < hue && hue < lowH)
    // {
    //     opacity = min(opacity, (hue - lowH+0.02) * 50.0);
    // }
    // if (highH < hue && hue < highH+0.02)
    // {
    //     opacity = min(opacity, (highH+0.02 - hue) * 50.0);
    // }
    // if (0.3 < value && value < 0.4)
    // {
    //     opacity = min(opacity, (0.4 - value) * 10.0);
    // }
    if ((0.1 <= hue && hue <= 0.89) || value <= 0.3) { 
        opacity = 0.0;
    }
    if (0.3 < value && value < 0.32) { 
        opacity = min(opacity, (0.32 - value) * 50.0);
    }

    vec3 color = vec3(opacity) * 0.5;
    gl_FragColor = vec4(clamp(color + 0.5, 0.0, 1.0), 1.0);
}