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
uniform float alpha;


void main()
{
    vec4 srcColor = texture2D(inputImageTexture, textureCoordinate);
    float alpha2 = srcColor.a;

    vec3 levelColor = (1.0 - alpha) * srcColor + alpha;
    gl_FragColor = vec4(levelColor, alpha2);
}