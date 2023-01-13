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

in vec3 Normal;
in vec3 WorldPos;
in vec2 TexCoords;
in vec3 eyeright;
in vec3 eyeup;

layout (location = 0) out vec4 out_FragColor;

uniform sampler2D albedoMap;
uniform sampler2D specularMap;
uniform sampler2D heightMap;

uniform samplerCube irradianceMap;
uniform samplerCube prefilterMap;
uniform sampler2D brdfLUT;
uniform mat4 RotateIBL;
// lights
uniform vec3 lightDir[4];
uniform vec3 lightColors[4];
uniform float lightStrength[4];
uniform int LightNum;
uniform float ambientStrength;

uniform vec3 camPos;

uniform float movespeed1;
uniform float movespeed2;   
uniform float movespeed3;
uniform float eyeheight;
   
uniform vec3 specoffset1; 
uniform vec3 specoffset2;
uniform vec3 specoffset3;
uniform vec3 initoffset;
uniform vec3 intensity;


//Function main 
void main()
{  
	vec3 vView = normalize(camPos - WorldPos);
	vec3 vLight = normalize(lightDir[0]);
	vLight.z= -vLight.z;

	float vr = dot(vView, eyeright);
    float vu = dot(vView, eyeup);

    vec2 coord0 = vec2(-vr*vr*vr, -vr);
    coord0 *= movespeed1;
    coord0 *= specoffset1.xy;

	coord0 = coord0 + initoffset.xy + vec2(TexCoords.x, 1.0 - TexCoords.y);
    coord0.y = 1.0 - coord0.y;

	float specular0 = texture(specularMap, coord0).x;

	vec2 coord1 = vec2(vr, -vu);
    coord1 *= movespeed2;
    coord1 *= specoffset2.xy;
    coord1 = coord1 + initoffset.xy + vec2(TexCoords.x, 1.0 - TexCoords.y);
    coord1.y = 1.0 - coord1.y;
    float specular1 = texture(specularMap, coord1).y;

	vec2 coord2 = abs(vec2(vr, vr));
    coord2 *= movespeed3;
    coord2 *= specoffset3.xy;
    coord2 = coord2 + initoffset.xy + vec2(TexCoords.x, 1.0 - TexCoords.y);
    coord2.y = 1.0 - coord2.y;

    float specular2 = texture(specularMap, coord2).z;

	float totalSpecular = dot(vec3(specular0,specular1,specular2), intensity);

	
    vec2 heightOffset = vec2(vr, -vu) * eyeheight * 0.5;
    float height0 = texture(heightMap, TexCoords).x;

    vec2 hcoord1 = vec2(height0) * heightOffset + vec2(TexCoords.x, 1.0 - TexCoords.y);
    hcoord1.y = 1.0 - hcoord1.y;

    float height1 = texture(heightMap, hcoord1).x;

    vec2 hcoord2 = vec2(height1,height1) * heightOffset + vec2(TexCoords.x, 1.0 - TexCoords.y);
    hcoord2.y = 1.0 - hcoord2.y;
    float height2 = texture(heightMap, hcoord2).x;

    vec2 hcoord3 = vec2(height2, height2) * heightOffset + vec2(TexCoords.x, 1.0 - TexCoords.y);
    hcoord3.y = 1.0 - hcoord3.y;
    vec3 mainColor = texture(albedoMap, hcoord3).xyz;
    mainColor *= mainColor;

    float NdotL = max(dot(normalize(Normal),vLight ), 0.0);  
    vec3 light = lightColors[0].xyz * lightStrength[0]*NdotL ;
	
    vec3 finalColor = mainColor + vec3(totalSpecular, totalSpecular, totalSpecular);
    finalColor *= light;

    out_FragColor = vec4(finalColor, 1.0);

}

