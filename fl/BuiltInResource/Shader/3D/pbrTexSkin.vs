#version 300 es
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2)in vec2 texCoords;
layout(location = 3)in vec4 gBoneIDsArray;
layout(location = 4) in vec4 gBoneWeightsArray;

out vec3 Normal;
out vec3 WorldPos;
out vec2 TexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
const int MAX_BONES = 200;
uniform mat4 gBonesMatrix[MAX_BONES];

void main()
{
	int i0 = int(gBoneIDsArray[0]);
	int i1 = int(gBoneIDsArray[1]);
	int i2 = int(gBoneIDsArray[2]);
	int i3 = int(gBoneIDsArray[3]);
	mat4 BoneTransform = gBonesMatrix[i0] * gBoneWeightsArray[0];	
	BoneTransform += gBonesMatrix[i1] * gBoneWeightsArray[1];
	BoneTransform += gBonesMatrix[i2] * gBoneWeightsArray[2];
	BoneTransform += gBonesMatrix[i3] * gBoneWeightsArray[3];
	vec4 ModelVertex = vec4(position, 1.0);
	mat4 ModeltoWorld=model* BoneTransform;
	vec4 posL = ModeltoWorld*ModelVertex;
	WorldPos = posL.xyz/posL.w;
	
	vec4 n = vec4(normal, 0.0);
	n =  ModeltoWorld*n;
	vec4 ns = vec4(0.0, 0.0, 0.0, 0.0);
	ns = ModeltoWorld*ns;
	Normal = normalize(n - ns).xyz;

	TexCoords = texCoords;
	gl_Position = projection *view* posL;
	gl_Position.x = -gl_Position.x;
	//gl_Position.z/=10.0;//
}