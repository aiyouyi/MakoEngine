#version 300 es
#ifdef GL_ES
precision highp  float;
#else
#define highp
#define mediump
#define lowp
#endif
in vec2 textureCoordinate;
uniform sampler2D inputImageTexture;
out vec4 out_FragColor;

vec3 ACESFilm(vec3 color)
{
	vec3 x = 0.8 * color;
	float a = 2.51;
	float b = 0.03;
	float c = 2.43;
	float d = 0.59;
	float e = 0.14;
	return clamp((x*(a*x + b)) / (x*(c*x + d) + e),0.0,1.0);
}
 
float LinearToSrgbChannel(float lin)
{
	if (lin < 0.00313067) return lin * 12.92;
	return pow(lin, (1.0 / 2.4)) * 1.055 - 0.055;
}

vec3 LinearToSrgb(vec3 lin)
{
	//return pow(lin, 1/2.2);
	return vec3(LinearToSrgbChannel(lin.r), LinearToSrgbChannel(lin.g), LinearToSrgbChannel(lin.b));
}

float SrgbToLinearChannel(float lin)
{
	if (lin < 0.04045) return lin/ 12.92;
	return pow(lin / 1.055 + 0.0521327,2.4);
}


vec3 sRGBToLinear(vec3 Color)
{
	Color = max(vec3(6.10352e-5), Color); // minimum positive non-denormal (fixes black problem on DX11 AMD and NV)
    return vec3(SrgbToLinearChannel(Color.r), SrgbToLinearChannel(Color.g), SrgbToLinearChannel(Color.b));
}

vec3 ToneMapping(vec3 Color)
{
	return LinearToSrgb(ACESFilm(Color));
}

void main()
{

    vec4 Color = texture(inputImageTexture,textureCoordinate);
	out_FragColor = vec4(ToneMapping(Color.rgb),Color.a);
}