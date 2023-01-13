#version 300 es
#ifdef GL_ES
precision highp  float;
#else
#define highp
#define mediump
#define lowp
#endif

in vec3 WorldPos;
out vec4 out_FragColor;
uniform sampler2D equirectangularMap;

vec2 invAtan = vec2(0.1591, 0.3183);

vec2 SampleSphericalMap(vec3 v)
{
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv *= invAtan;
    uv += 0.5;
    return uv;
}
void main()
{
    vec2 uv = SampleSphericalMap(normalize(WorldPos));
    vec3 color = texture(equirectangularMap, uv).rgb;
    out_FragColor = vec4(color, 1.0);
}
