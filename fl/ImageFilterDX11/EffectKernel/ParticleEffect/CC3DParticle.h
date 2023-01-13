#ifndef _CC_PARTICLE_H_
#define _CC_PARTICLE_H_

#include "Common/CC3DUtils.h"
#include "BaseDefine/Vectors.h"

class Particle
{
public:
	Particle();
	virtual ~Particle();
	glm::vec3 Position;
	glm::vec3 StartPosition;
	glm::vec3 EndPosition;
	glm::vec3 ControlPosition;
	glm::vec2 texcoord;
	glm::vec2 Velocity;
	glm::vec4 ColorBegin, ColorEnd;
	float Rotation = 0.0f;
	float SizeBegin, SizeEnd;

	float LifeTime = 3.0f;
	float LifeRemaining = 0.0f;

	Vector4 faceOri;
	glm::mat4 projMat = glm::mat4(1.0f);

	bool Active = false;
};

class BubbleParticle : public Particle
{
public:
	BubbleParticle(): Particle()
	{}
	virtual ~BubbleParticle() {}

	virtual void Update(float ts);
};

class FireParticle : public Particle
{
public:
	FireParticle() : Particle()
	{}
	virtual ~FireParticle() {}

	virtual void Update(float ts);
};

#endif