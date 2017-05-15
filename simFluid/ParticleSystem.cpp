#include "ParticleSystem.h"

using namespace std;

const int WORK_GROUP_SIZE = 256;
static unsigned int mirand = 1;

float sfrand(void)
{
	unsigned int a;
	mirand *= 16807;
	a = (mirand & 0x007fffff) | 0x40000000;
	return((*((float*)&a) - 3.0f));
}

void _check_gl_error(const char *file, int line) {
	GLenum err(glGetError());

	while (err != GL_NO_ERROR) {
		string error;

		switch (err) {
		case GL_INVALID_OPERATION:      error = "INVALID_OPERATION";      break;
		case GL_INVALID_ENUM:           error = "INVALID_ENUM";           break;
		case GL_INVALID_VALUE:          error = "INVALID_VALUE";          break;
		case GL_OUT_OF_MEMORY:          error = "OUT_OF_MEMORY";          break;
		case GL_INVALID_FRAMEBUFFER_OPERATION:  error = "INVALID_FRAMEBUFFER_OPERATION";  break;
		}

		cerr << "GL_" << error.c_str() << " - " << file << ":" << line << endl;
		err = glGetError();
	}
}

ParticleSystem::ParticleSystem(size_t partSize) : size(partSize)
{
	index = new ShaderBuffer<uint32_t>(size);
	gridIndex = new ShaderBuffer<uint32_t>(size);

	nodes = new ShaderBuffer<Node>(GRIDX*GRIDY);
	particles = new ShaderBuffer<Particle>(size);

	scaletowindow = 1.0;

	uint32_t *indices = index->map();
	for (size_t i = 0; i<size; i++) {
		*(indices++) = i;
	}
	index->unmap();

	indices = gridIndex->map();
	for (size_t i = 0; i<size; i++) {
		*(indices++) = i;
	}
	gridIndex->unmap();

	loadShaders();
	initialize();
	
}


ParticleSystem::~ParticleSystem()
{
	delete particles;
	delete nodes;

	//glDeleteProgram(updateProg);
	glDeleteProgram(ProgramPass1);
	glDeleteProgram(ProgramNode1);
	glDeleteProgram(ProgramPass2);
	glDeleteProgram(ProgramNode2);
	glDeleteProgram(ProgramPass3);
	glDeleteProgram(ProgramNode3);
	glDeleteProgram(ProgramPass4);
}

void ParticleSystem::loadComputeShader(string filename, GLuint *ProgramID, GLuint *PipelineID) {

	if (*ProgramID) {
		glDeleteProgram(*ProgramID);
		*ProgramID = 0;
	}

	// Read uniforms file
	string uniformsCode;
	ifstream uniformsStream("uniforms.h", ios::in);
	if (uniformsStream.is_open()) {
		string Line = "";
		while (getline(uniformsStream, Line))
			uniformsCode += "\n" + Line;
		uniformsStream.close();
	}
	else {
		getchar();
	}

	// Read the Compute Shader code from the file
	string ComputeShaderCode;
	ifstream ComputeShaderStream(filename, ios::in);
	if (ComputeShaderStream.is_open()) {
		string Line = "";
		while (getline(ComputeShaderStream, Line))
			ComputeShaderCode += "\n" + Line;
		ComputeShaderStream.close();
	}
	else {
		getchar();
	}
	size_t uniformTagPos = ComputeShaderCode.find("#UNIFORMS");
	string dest = "";
	if (uniformTagPos != string::npos) {
		dest += ComputeShaderCode.substr(0, uniformTagPos); // source up to tag
		dest += "\n";
		dest += uniformsCode;
		dest += "\n";
		dest += ComputeShaderCode.substr(uniformTagPos + strlen("#UNIFORMS"), ComputeShaderCode.length() - uniformTagPos);
	}
	else {
		dest += ComputeShaderCode;
	}
	ComputeShaderCode = dest;


	const GLchar* src[1] = { ComputeShaderCode.c_str() };
	//cout << ComputeShaderCode.c_str() << endl;

	// Create the shaders
	*ProgramID = glCreateShaderProgramv(GL_COMPUTE_SHADER, 1, src);
	GLint Result = GL_FALSE;
	int InfoLogLength;
	// Check the program

	glGenProgramPipelines(1, PipelineID);

	glGetProgramiv(*ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		char *log = new char[InfoLogLength];
		glGetProgramInfoLog(*ProgramID, InfoLogLength, 0, log);
		printf("Shader pipeline program not valid:\n%s\n", log);
		delete[] log;

	}


	glBindProgramPipeline(*PipelineID);
	glUseProgramStages(*PipelineID, GL_COMPUTE_SHADER_BIT, *ProgramID);
	glValidateProgramPipeline(*PipelineID);
	glGetProgramPipelineiv(*PipelineID, GL_VALIDATE_STATUS, &Result);

	if (Result != GL_TRUE) {
		GLint InfoLogLength;
		glGetProgramPipelineiv(*PipelineID, GL_INFO_LOG_LENGTH, &InfoLogLength);
		char *log = new char[InfoLogLength];
		glGetProgramPipelineInfoLog(*PipelineID, InfoLogLength, 0, log);
		printf("Shader pipeline not valid:\n%s\n", log);
		delete[] log;
	}

	//glBindProgramPipeline(progPipeline);
	glBindProgramPipeline(0);
}

void ParticleSystem::loadShaders()
{
	loadComputeShader("computepass1.glsl", &ProgramPass1, &PipelinePass1);
	/*loadComputeShader("computenode1.glsl", &ProgramNode1, &PipelineNode1);
	loadComputeShader("computepass2.glsl", &ProgramPass2, &PipelinePass2);
	loadComputeShader("computenode2.glsl", &ProgramNode2, &PipelineNode2);
	loadComputeShader("computepass3.glsl", &ProgramPass3, &PipelinePass3);
	loadComputeShader("computenode3.glsl", &ProgramNode3, &PipelineNode3);
	loadComputeShader("computepass4.glsl", &ProgramPass4, &PipelinePass4);*/
}

void ParticleSystem::initialize()
{
	Particle *particle = particles->map();
	for (size_t i = 0; i<size; i++) {
		particle[i].x = sfrand()*scaletowindow;
		particle[i].y = sfrand()*scaletowindow;
		particle[i].u = particle[i].v = particle[i].pu = particle[i].pv = particle[i].d = particle[i].gu = particle[i].gv = particle[i].T00 = particle[i].T01 = particle[i].T11 = 0;
		particle[i].cx = particle[i].cy = 0;
		particle[i].px[0] = particle[i].px[1] = particle[i].px[2] = particle[i].py[0] = particle[i].py[1] = particle[i].py[2] = particle[i].gx[0] = particle[i].gx[1] = particle[i].gx[2] = particle[i].gy[0] = particle[i].gy[1] = particle[i].gy[2] = 0;
		//cout << particle[i].x << endl;
	}
	particles->unmap();

}

void ParticleSystem::update()
{
	// Invoke the compute shader to integrate the particles
	glBindProgramPipeline(PipelinePass1);

	glActiveTexture(GL_TEXTURE0);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, particles->getBuffer());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, nodes->getBuffer());
	
	glDispatchCompute(COMPUTESIZE, 1, 1);

	// We need to block here on compute completion to ensure that the
	// computation is done before we render
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, 0);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);
	/*
	glBindProgramPipeline(PipelineNode1);

	glActiveTexture(GL_TEXTURE0);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, particles->getBuffer());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, nodes->getBuffer());

	glDispatchCompute(COMPUTESIZE, 1, 1);

	// We need to block here on compute completion to ensure that the
	// computation is done before we render
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, 0);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);
	
	glBindProgramPipeline(PipelinePass2);

	glActiveTexture(GL_TEXTURE0);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, particles->getBuffer());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, nodes->getBuffer());

	glDispatchCompute(COMPUTESIZE, 1, 1);

	// We need to block here on compute completion to ensure that the
	// computation is done before we render
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, 0);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);

	glBindProgramPipeline(PipelineNode2);

	glActiveTexture(GL_TEXTURE0);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, particles->getBuffer());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, nodes->getBuffer());

	glDispatchCompute(COMPUTESIZE, 1, 1);

	// We need to block here on compute completion to ensure that the
	// computation is done before we render
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, 0);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);

	glBindProgramPipeline(PipelinePass3);

	glActiveTexture(GL_TEXTURE0);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, particles->getBuffer());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, nodes->getBuffer());

	glDispatchCompute(COMPUTESIZE, 1, 1);

	// We need to block here on compute completion to ensure that the
	// computation is done before we render
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, 0);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);

	glBindProgramPipeline(PipelineNode3);

	glActiveTexture(GL_TEXTURE0);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, particles->getBuffer());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, nodes->getBuffer());

	glDispatchCompute(COMPUTESIZE, 1, 1);

	// We need to block here on compute completion to ensure that the
	// computation is done before we render
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, 0);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);

	glBindProgramPipeline(PipelinePass4);

	glActiveTexture(GL_TEXTURE0);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, particles->getBuffer());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, nodes->getBuffer());

	glDispatchCompute(COMPUTESIZE, 1, 1);

	// We need to block here on compute completion to ensure that the
	// computation is done before we render
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, 0);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);
	*/
	glBindProgramPipeline(0);
}
