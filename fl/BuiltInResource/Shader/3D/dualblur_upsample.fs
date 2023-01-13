#version 300 es

#ifdef GL_ES//for discriminate GLES & GL
#ifdef GL_FRAGMENT_PRECISION_HIGH
precision highp float;
#else
precision highp float;
#endif
#else
#define highp
#define mediump
#define lowp
#endif

uniform sampler2D MainTex;

in vec2 uv;
in vec4 uv01;
in vec4 uv23;
in vec4 uv45;
in vec4 uv67;
layout (location = 0) out vec4 out_FragColor;

void main()
{
	vec4 sum = vec4(0.0);
	sum += texture( MainTex, uv01.xy );
	sum += texture( MainTex, uv01.zw ) * 2.0;
	sum += texture( MainTex, uv23.xy );
	sum += texture( MainTex, uv23.zw ) * 2.0;
	sum += texture( MainTex, uv45.xy );
	sum += texture( MainTex, uv45.zw ) * 2.0;
	sum += texture( MainTex, uv67.xy );
	sum += texture( MainTex, uv67.zw ) * 2.0;

	out_FragColor = sum * 0.0833;
}