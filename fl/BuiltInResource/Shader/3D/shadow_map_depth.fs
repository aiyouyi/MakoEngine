#version 300 es
#ifdef GL_ES
precision highp  float;
#else
#define highp
#define mediump
#define lowp
#endif

out vec3 outColor;	
uniform float alpha;
void main()
{             
	 float depth = (gl_FragCoord.z);
	 outColor = vec3(depth,depth * depth,alpha);
}