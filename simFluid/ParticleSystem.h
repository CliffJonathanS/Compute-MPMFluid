#ifndef PARTICLE_SYSTEM_H
#define PARTICLE_SYSTEM_H

// Include OpenGL dependencies
#include "Dependencies\glew\glew.h"
#include "Dependencies\freeglut\freeglut.h"

// Include dependency
#include "ShaderBuffer.h"
#include "vec4f.h"

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
class ParticleSystem
{
public:
	ParticleSystem(size_t size);
	~ParticleSystem();

	void loadShaders(void);
	void initialize(void);
	void update(void);

	ShaderBuffer<vec4f> *getPosBuffer() { return pos; }
	ShaderBuffer<vec4f> *getVelBuffer() { return vel; }
	ShaderBuffer<uint32_t> *getIndexBuffer() { return index; }
	size_t getSize() { return size; }

private:
	size_t size;

	ShaderBuffer<vec4f> *pos;
	ShaderBuffer<vec4f> *vel;
	ShaderBuffer<uint32_t> *index;

	GLuint updateProg;
	GLuint progPipeline;
};
#endif