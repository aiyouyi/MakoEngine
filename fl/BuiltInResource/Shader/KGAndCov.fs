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
uniform sampler2D u_recordTexture0;
uniform sampler2D u_recordTexture1;
uniform sampler2D u_recordTexture2;
uniform sampler2D u_recordTexture3;
uniform sampler2D u_recordTexture4;
uniform sampler2D u_recordTexture5;
uniform sampler2D u_lastCovTexture;
void main()
{
    vec4 vMax = texture2D(u_recordTexture0, textureCoordinate);
    vec4 vMin = vMax;
    vec4 vColor = texture2D(u_recordTexture1, textureCoordinate);
    vMax = max(vMax, vColor);
    vMin = min(vMin, vColor);
    vColor = texture2D(u_recordTexture2, textureCoordinate);
    vMax = max(vMax, vColor);
    vMin = min(vMin, vColor);
    vColor = texture2D(u_recordTexture3, textureCoordinate);
    vMax = max(vMax, vColor);
    vMin = min(vMin, vColor);
    vColor = texture2D(u_recordTexture4, textureCoordinate);
    vMax = max(vMax, vColor);
    vMin = min(vMin, vColor);
    vColor = texture2D(u_recordTexture5, textureCoordinate);
    vMax = max(vMax, vColor);
    vMin = min(vMin, vColor);
    float fChange = (vMax.x - vMin.x)/6.0;
    float fMeasureCov = 50.0/255.0;
    float fPredictionCov = max(fMeasureCov*fChange*255.0/5.0, fMeasureCov/1.5);
    float fFilterCov = texture2D(u_lastCovTexture, textureCoordinate).x;
    float fPreConv = fPredictionCov*fPredictionCov + fFilterCov*fFilterCov;
    float fMesCov = fMeasureCov*fMeasureCov;
    float fKG = sqrt(fPreConv/(fPreConv+fMesCov));
    fFilterCov = sqrt((1.0-fKG)*fPreConv);
    gl_FragColor = vec4(fFilterCov, fKG, fChange, 1.0);
}