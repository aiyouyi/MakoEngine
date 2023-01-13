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
varying  vec2 textureCoordinate;

uniform sampler2D inputImageTexture;
uniform  vec2 stepSize;

vec3 rgb2hsv(vec3 rgb)
{
    vec4 N = vec4(0.0, -0.33333, 0.66667, -1.0);
    vec4 f1 = mix(vec4(rgb.bg, N.wz), vec4(rgb.gb, N.xy), step(rgb.b, rgb.g));
    vec4 f2 = mix(vec4(f1.xyw, rgb.r), vec4(rgb.r, f1.yzx), step(f1.x, rgb.r));
    vec3 hsv = vec3(abs(f2.z + (f2.w - f2.y) / (6.0 * (f2.x - min(f2.w, f2.y)) + 0.0000000001)), 0.0, f2.x);

    return hsv;
}

void main()
{
    vec4 iColor = texture2D(inputImageTexture, textureCoordinate);

    vec3 sum = iColor.rgb * 0.5;
    sum += texture2D(inputImageTexture, textureCoordinate + vec2(0.5, 1.0) * stepSize).rgb;
    sum += texture2D(inputImageTexture, textureCoordinate + vec2(1.0, -0.5) * stepSize).rgb;
    sum += texture2D(inputImageTexture, textureCoordinate + vec2(-0.5, -1.0) * stepSize).rgb;
    sum += texture2D(inputImageTexture, textureCoordinate + vec2(-1.0, 0.5) * stepSize).rgb;

    vec3 hsvColor = rgb2hsv(iColor.rgb);
    float skinAlpha = clamp((hsvColor.z - 0.2) * 10.0, 0.0, 1.0);
    skinAlpha = skinAlpha * clamp(max((0.18 - hsvColor.x) * 50.0, (hsvColor.x - 0.89) * 50.0), 0.0, 1.0);
    skinAlpha = clamp(skinAlpha + 0.5, 0.0, 1.0);

    gl_FragColor = vec4(sum * 0.22222, skinAlpha);
}