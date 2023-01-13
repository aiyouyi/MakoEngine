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
uniform sampler2D inputImageTextureFace;
varying  vec2 textureCoordinate;
uniform  float texelWidthOffset;
uniform  float texelHeightOffset;
uniform  float alpha;

void main()
{
    vec4 meanColor = vec4(0.0);
    vec4 corrColor = vec4(0.0);
    vec4 face = texture2D(inputImageTextureFace, textureCoordinate);
    float faceAlpha = alpha/face.g;
    float WidthOffset = texelWidthOffset*face.g;
    float HeightOffest = texelHeightOffset*face.g;

    vec4 color = texture2D(inputImageTexture, textureCoordinate + vec2(-WidthOffset, -HeightOffest));
    color.a = dot(color.rgb, vec3(0.2989, 0.5870, 0.1140));
    meanColor += color;
    corrColor += color.a * color;
    color = texture2D(inputImageTexture, textureCoordinate + vec2(-WidthOffset, 0.0));
    color.a = dot(color.rgb, vec3(0.2989, 0.5870, 0.1140));
    meanColor += color;
    corrColor += color.a * color;
    color = texture2D(inputImageTexture, textureCoordinate + vec2(-WidthOffset, HeightOffest));
    color.a = dot(color.rgb, vec3(0.2989, 0.5870, 0.1140));
    meanColor += color;
    corrColor += color.a * color;
    color = texture2D(inputImageTexture, textureCoordinate + vec2(0.0, -HeightOffest));
    color.a = dot(color.rgb, vec3(0.2989, 0.5870, 0.1140));
    meanColor += color;
    corrColor += color.a * color;
    color = texture2D(inputImageTexture, textureCoordinate + vec2(0.0, 0.0));
    color.a = dot(color.rgb, vec3(0.2989, 0.5870, 0.1140));
    meanColor += color;
    corrColor += color.a * color;
    vec4 SrcColor = color;

    color = texture2D(inputImageTexture, textureCoordinate + vec2(0.0, HeightOffest));
    color.a = dot(color.rgb, vec3(0.2989, 0.5870, 0.1140));
    meanColor += color;
    corrColor += color.a * color;
    color = texture2D(inputImageTexture, textureCoordinate + vec2(WidthOffset, -HeightOffest));
    color.a = dot(color.rgb, vec3(0.2989, 0.5870, 0.1140));
    meanColor += color;
    corrColor += color.a * color;
    color = texture2D(inputImageTexture, textureCoordinate + vec2(WidthOffset, 0.0));
    color.a = dot(color.rgb, vec3(0.2989, 0.5870, 0.1140));
    meanColor += color;
    corrColor += color.a * color;
    color = texture2D(inputImageTexture, textureCoordinate + vec2(WidthOffset, HeightOffest));
    color.a = dot(color.rgb, vec3(0.2989, 0.5870, 0.1140));
    meanColor += color;
    corrColor += color.a * color;

    meanColor *= 0.111111;
    corrColor *= 0.111111;
    vec4 varColor = corrColor - meanColor.a * meanColor;

    float a = max(0.0, varColor.a / (varColor.a + faceAlpha * faceAlpha));
    a = 1.0 - (1.0-a)*(face.r);
    vec3 b = SrcColor.rgb - a * SrcColor.rgb;
    gl_FragColor = vec4(b, a);
}