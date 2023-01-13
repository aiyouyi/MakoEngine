#version 300 es
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoords;
layout(location = 3) in vec4 tangent;
layout(location = 4) in vec4 gBoneIDsArray;
layout(location = 5) in vec4 gBoneWeightsArray;


out vec3 Normal;
out vec3 WorldPos;
out vec2 TexCoords;
out vec2 EmissUV;
out vec4 FragPosLightSpace;
out vec4 Tangent;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 meshMat;
uniform mat4 lightSpaceMatrix;
const int MAX_BONES = 200;
uniform mat4 gBonesMatrix[MAX_BONES];

uniform bool shadowsEnable;
uniform bool AnimationEnable;
uniform float _time;

void main()
{
	mat4 modelToWorld = mat4(1.0);

	if (AnimationEnable)
	{
		int i0 = int(gBoneIDsArray[0]);
        int i1 = int(gBoneIDsArray[1]);
        int i2 = int(gBoneIDsArray[2]);
        int i3 = int(gBoneIDsArray[3]);
        mat4 BoneTransform = gBonesMatrix[i0] * gBoneWeightsArray[0];	
        BoneTransform += gBonesMatrix[i1] * gBoneWeightsArray[1];
        BoneTransform += gBonesMatrix[i2] * gBoneWeightsArray[2];
        BoneTransform += gBonesMatrix[i3] * gBoneWeightsArray[3];
        modelToWorld =  BoneTransform * inverse(meshMat);
	}

	modelToWorld = model * modelToWorld * meshMat;
	vec4 posW = modelToWorld * vec4(position, 1.0);
	WorldPos = posW.xyz / posW.w;


	vec4 n = vec4(normal, 0.0);
	n =  modelToWorld * n;
	vec4 ns = vec4(0.0, 0.0, 0.0, 0.0);
	ns = modelToWorld * ns;
	Normal = normalize(n - ns).xyz;
	
	if (shadowsEnable)
	{
		FragPosLightSpace = lightSpaceMatrix * posW;
	}

	TexCoords = texCoords;
	EmissUV = vec2(texCoords.x + _time, texCoords.y);

	vec4 t = modelToWorld * tangent;
	vec4 ts = vec4(0.0, 0.0, 0.0, 0.0);
	ts = modelToWorld * ts;
	Tangent =  t - ts ;

	vec4 tan = modelToWorld * vec4( tangent.xyz, 0.0);
	Tangent = vec4( tan.xyz, tangent.w );

	gl_Position = projection *view* posW;

}