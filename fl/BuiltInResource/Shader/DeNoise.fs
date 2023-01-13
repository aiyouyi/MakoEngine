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
uniform sampler2D u_measureImageTexture;
uniform sampler2D u_predictionImageTexture;
uniform sampler2D u_filterWeightImageTexture;
void main()
{
    vec4 measureColor = texture2D(u_measureImageTexture, textureCoordinate);
    vec4 predictColor = texture2D(u_predictionImageTexture, textureCoordinate);
    float fFilterWeight = texture2D(u_filterWeightImageTexture, textureCoordinate).y;
    gl_FragColor = predictColor + (measureColor-predictColor)*fFilterWeight;
}