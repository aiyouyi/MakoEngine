#version 300 es
#ifdef GL_ES
precision highp  float;
#else
#define highp
#define mediump
#define lowp
#endif

out vec4 out_FragColor;

void main()
{
    out_FragColor = vec4(1.0,1.0,1.0,1.0);
}