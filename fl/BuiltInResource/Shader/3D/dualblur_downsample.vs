#version 300 es
layout(location = 0) in vec3 position;

uniform vec2 mainTex_texelSize; //从外部传入的时候需要乘以0.5
uniform float _Offset;

out vec2 uv;
out vec4 uv01;
out vec4 uv23;

void main() 
{
    gl_Position = vec4(position,1.0);
    uv = position.xy * 0.5 + 0.5;

	//mainTex_texelSize 
	uv01.xy = uv - mainTex_texelSize * vec2( 1.0 + _Offset, 1.0 + _Offset);
	uv01.zw = uv + mainTex_texelSize * vec2( 1.0 + _Offset, 1.0 + _Offset);

	uv23.xy = uv - vec2( mainTex_texelSize.x, -mainTex_texelSize.y ) * vec2( 1.0 + _Offset, 1.0 + _Offset );
	uv23.zw = uv + vec2( mainTex_texelSize.x, -mainTex_texelSize.y ) * vec2( 1.0 + _Offset, 1.0 + _Offset );
}