// Include OpenGL dependencies
#include "Dependencies\glew\glew.h"
#include "Dependencies\freeglut\freeglut.h"

// Include dependency
#include "ShaderBuffer.h"

// Include headers
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

/*
	Class		: ParticleSystem
	Description	: The particle system class which initializes and runs the Compute Shader
				  program to calculate the particle positions for each frame.
*/
#pragma once
class ParticleSystem
{
public:
	ParticleSystem(size_t size);
	~ParticleSystem();

	void loadShaders();
	void initialize();
	void update();

private:
	size_t size;

	ShaderBuffer<float[4]> *pos;
	ShaderBuffer<float[4]> *vel;
	ShaderBuffer<uint32_t> *index;

	GLuint updateProg;
	GLuint progPipeline;
};

