#ifndef PARTICLE_SYSTEM_H
#define PARTICLE_SYSTEM_H

// Include OpenGL dependencies
#include "Dependencies\glew\glew.h"
#include "Dependencies\freeglut\freeglut.h"

// Include dependency
#include "ShaderBuffer.h"
#include "vec4f.h"
#include "uniforms.h"

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

	void loadComputeShader(std::string filename, GLuint *ProgramID, GLuint *PipelineID);
	void loadShaders(void);
	void initialize(void);
	void update(void);

	ShaderBuffer<uint32_t> *getIndexBuffer() { return index; }
	ShaderBuffer<Particle> *getParticlesBuffer() { return particles; }
	ShaderBuffer<Node> *getNodesBuffer() { return nodes; }
	size_t getSize() { return size; }

private:
	size_t size;

	ShaderBuffer<uint32_t> *index;
	ShaderBuffer<uint32_t> *gridIndex;

	ShaderBuffer<Node> *nodes;
	ShaderBuffer<Particle> *particles;

	GLuint updateProg;

	GLuint PipelinePass1;
	GLuint PipelineNode1;
	GLuint PipelinePass2;
	GLuint PipelineNode2;
	GLuint PipelinePass3;
	GLuint PipelineNode3;
	GLuint PipelinePass4;

	GLuint ProgramPass1;
	GLuint ProgramNode1;
	GLuint ProgramPass2;
	GLuint ProgramNode2;
	GLuint ProgramPass3;
	GLuint ProgramNode3;
	GLuint ProgramPass4;

	float scaletowindow;
};
#endif