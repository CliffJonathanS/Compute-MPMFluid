#pragma once
class ParticleSystem
{
public:
	ParticleSystem();
	~ParticleSystem();

	void loadShaders();
	void initialize();
	void update();
};

