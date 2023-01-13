#version 300 es

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoordDiffuse;
layout(location = 3) in vec4 tangent;
layout(location = 4) in vec4 gBoneIDsArray;
layout(location = 5) in vec4 gBoneWeightsArray;


out vec3 out_pos;
out vec2 T0;
out vec2 T1;
out vec3 out_normal;
out vec3 SH;
out vec4 Tangent;

uniform float FurLength;
uniform float UVScale;
uniform float Force;
uniform float FurOffset;

uniform vec3 vGravity;
//uniform float vGravityStength;
uniform vec4 vecLightDir;

uniform mat4 world;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 meshMat;
const int MAX_BONES = 100;
uniform mat4 gBonesMatrix[MAX_BONES];
//uniform sampler2D lengthTexture;

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

	vec2 uvoffset = vec2(0.2, 0.2) * FurOffset;
	uvoffset *= 0.1;

	T0 = texCoordDiffuse * UVScale + uvoffset;
	T1 = texCoordDiffuse;

	//float furLength_coeff = texture(lengthTexture, T1).a;
	//furLength_coeff = 1.0f;
	vec3 P = position + normal * FurLength * FurOffset;

	P += vGravity * pow(FurOffset, 3.0) * FurLength;

//	vec3 direction = vGravity * vGravityStength + normal * (1.0 - vGravityStength);
//	direction = mix( normal, direction, FurOffset);
//	 P += direction * FurLength;


	//normal
	vec4 n = vec4(normal, 0.0);
	n =  modelToWorld * n;
	vec4 ns = vec4(0.0, 0.0, 0.0, 0.0);
	ns = modelToWorld * ns;
	out_normal = normalize(n - ns).xyz;

	vec4 view_normal = view * vec4(out_normal, 1.0);
	SH = vec3( clamp( out_normal.y * 0.25 + 0.35, 0.0, 1.0 ));

	gl_Position = projection * view * modelToWorld * vec4(P, 1.0);
	//gl_Position.x = -gl_Position.x;
	//out_pos = gl_Position.xyz;

	out_pos = (modelToWorld * vec4(P, 1.0)).xyz;

	vec4 tan = modelToWorld * vec4( tangent.xyz, 0.0);
	Tangent = vec4( tan.xyz, tangent.w );
}