#ifndef COMPUTE_MAIN_H
#define COMPUTE_MAIN_H
// Include OpenGL dependencies
#include "Dependencies\glew\glew.h"
#include "Dependencies\freeglut\freeglut.h"

// Include the particle system class
#include "ParticleSystem.h"

// Include headers
#include <iostream>
#include <fstream>
#include <string>
#include <vector>

/*
	Class		: ComputeMain
	Description	: Main class used to set up OpenGL environment and create the window
				  to run the fluid simulation.
*/
class ComputeMain
{
public:
	// ctor & dtor
	ComputeMain(int argc, char **argv);
	~ComputeMain();

	// Methods
	void update(void);
	void initRendering(void);
	int readFile(const char* filename, GLchar** ShaderSource, unsigned long* len);

private:
	ParticleSystem *particles;
	GLuint mVBO;
};

ComputeMain* currInstance;

extern "C"
void draw()
{
	currInstance->update();
}

#endif