#include "Dependencies\glew\glew.h"
#include "Dependencies\freeglut\freeglut.h"

#include <iostream>

#pragma once
class ComputeMain
{
public:
	ComputeMain(int argc, char **argv);
	~ComputeMain();
	static void update();
};

