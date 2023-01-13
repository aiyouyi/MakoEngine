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
out vec4 out_FragColor;



void main()
{
	vec4 sum = texture( MainTex, uv ) * 4.0;
	sum += texture( MainTex, uv01.xy );
	sum += texture( MainTex, uv01.zw );
	sum += texture( MainTex, uv23.xy );
	sum += texture( MainTex, uv23.zw );

	out_FragColor = sum * 0.125;
}