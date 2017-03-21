/*
	Class		: ParticleSystem
	Description	: The particle system class which initializes and runs the Compute Shader
				  program to calculate the particle positions for each frame.
*/

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

