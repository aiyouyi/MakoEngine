#include "CC3DParticle.h"

Particle::Particle()
{

}

Particle::~Particle()
{

}

void BubbleParticle::Update(float ts)
{
	LifeRemaining -= ts;
	Position.x += Velocity.r * (float)ts;
	Position.y += Velocity.g * (float)ts;
	Rotation += 1.0f * ts;
}

void FireParticle::Update(float ts)
{
	//LifeRemaining -= ts;
	//float tRatio = 1.0 - LifeRemaining * 1.0 / LifeTime;
	//float f1, f2, f3;
	//f1 = (1.0 - tRatio) * (1.0 - tRatio);
	//f2 = 2 * tRatio * (1.0 - tRatio);
	//f3 = tRatio * tRatio;
	//Position = f1 * StartPosition + f2 * ControlPosition + f3 * EndPosition;

	//Rotation += 1.0f * ts;

	LifeRemaining -= ts;
	Position.x += Velocity.r * (float)ts;
	Position.y += Velocity.g * (float)ts;
	Rotation += 1.0f * ts;
}
