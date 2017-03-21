// Include OpenGL dependencies
#include "Dependencies\glew\glew.h"
#include "Dependencies\freeglut\freeglut.h"

// Include the particle system class
#include "ParticleSystem.h"

// Include dependency for I/O
#include <iostream>

/*
	Class		: ComputeMain
	Description	: Main class used to set up OpenGL environment and create the window
				  to run the fluid simulation.
*/
#pragma once
class ComputeMain
{
public:
	// ctor & dtor
	ComputeMain(int argc, char **argv);
	~ComputeMain();

	// Methods
	static void update();
};

