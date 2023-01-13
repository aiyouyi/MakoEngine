#version 300 es
layout(location = 0) in vec3 position;

uniform vec2 mainTex_texelSize; //从外部传入的时候需要乘以0.5
uniform float _Offset;

out vec2 uv;
out vec4 uv01;
out vec4 uv23;
out vec4 uv45;
out vec4 uv67;

void main() 
{
    gl_Position = vec4(position,1.0);
    uv = position.xy * 0.5 + 0.5;

	vec2 offset = vec2( 1.0 + _Offset, 1.0 + _Offset );

	uv01.xy = uv + vec2( -mainTex_texelSize.x * 2.0, 0.0 ) * offset;
	uv01.zw = uv + vec2( -mainTex_texelSize.x, mainTex_texelSize.y ) * offset;

	uv23.xy = uv + vec2( 0.0, mainTex_texelSize.y * 2.0 ) * offset;
	uv23.zw = uv + mainTex_texelSize * offset;

	uv45.xy = uv + vec2(mainTex_texelSize.x * 2.0, 0.0) * offset;
	uv45.zw = uv + vec2(mainTex_texelSize.x, -mainTex_texelSize.y) * offset;

	uv67.xy = uv + vec2(0, -mainTex_texelSize.y * 2.0) * offset;
	uv67.zw = uv - mainTex_texelSize * offset;
}