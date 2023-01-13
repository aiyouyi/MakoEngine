#version 300 es
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoords;
layout(location = 3) in vec4 tangent;
layout(location = 4) in vec4 gBoneIDsArray;
layout(location = 5) in vec4 gBoneWeightsArray;

uniform mat4 lightSpaceMatrix;
uniform mat4 world;
uniform mat4 meshMat;
const int MAX_BONES = 100;
uniform mat4 gBonesMatrix[MAX_BONES];
uniform bool AnimationEnable;

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
        modelToWorld =  world*BoneTransform;
	}
    else
    {
        modelToWorld = world * meshMat;
    }
    vec4 worldPos = modelToWorld * vec4(position, 1.0f);

//	vec4 n = vec4(normal, 0.0);
//	n = modelToWorld*n;
//	vec4 ns = vec4(0.0, 0.0, 0.0, 0.0);
//	ns = modelToWorld*ns;
//	vec3 WorldNormal =(n - ns).xyz;
//	WorldNormal = normalize(WorldNormal);
//
//    float bias = -0.01;
//	float NoL = clamp( dot(WorldNormal, -lightDir), 0.0, 1.0 );
//	float sinTheta = sqrt(1.0 - NoL * NoL);
//	vec3 offsetPosition = worldPos.xyz / worldPos.w + WorldNormal * (sinTheta * bias);
//
//    gl_Position = lightSpaceMatrix * vec4(offsetPosition, 1.0);

	gl_Position = lightSpaceMatrix * worldPos;
}