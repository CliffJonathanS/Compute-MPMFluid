#include "ParticleSystem.h"

using namespace std;

const int WORK_GROUP_SIZE = 128;
static unsigned int mirand = 1;

float sfrand(void)
{
	unsigned int a;
	mirand *= 16807;
	a = (mirand & 0x007fffff) | 0x40000000;
	return((*((float*)&a) - 3.0f));
}

ParticleSystem::ParticleSystem(size_t partSize) : size(partSize)
{
	pos = new ShaderBuffer<vec4f>(size);
	vel = new ShaderBuffer<vec4f>(size);
	index = new ShaderBuffer<uint32_t>(size * 6);

	uint32_t *indices = index->map();
	for (size_t i = 0; i<size; i++) {
		uint32_t j = uint32_t(i << 2);
		*(indices++) = j;
		*(indices++) = j + 1;
		*(indices++) = j + 2;
		*(indices++) = j;
		*(indices++) = j + 2;
		*(indices++) = j + 3;
	}
	index->unmap();

	loadShaders();
	initialize();
}


ParticleSystem::~ParticleSystem()
{
	delete pos;
	delete vel;

	glDeleteProgram(updateProg);
}

void ParticleSystem::loadShaders()
{
	if (updateProg) {
		glDeleteProgram(updateProg);
		updateProg = 0;
	}

	glGenProgramPipelines(1, &progPipeline);

	// Create the shaders
	GLuint ComputeShaderID = glCreateShader(GL_COMPUTE_SHADER);

	// Read the Compute Shader code from the file
	string ComputeShaderCode;
	ifstream ComputeShaderStream("compute.glsl", ios::in);
	if (ComputeShaderStream.is_open()) {
		string Line = "";
		while (getline(ComputeShaderStream, Line))
			ComputeShaderCode += "\n" + Line;
		ComputeShaderStream.close();
	}
	else {
		getchar();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Compute Shader
	char const * ComputeSourcePointer = ComputeShaderCode.c_str();
	glShaderSource(ComputeShaderID, 1, &ComputeSourcePointer, NULL);
	glCompileShader(ComputeShaderID);

	// Check Compute Shader
	glGetShaderiv(ComputeShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(ComputeShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		vector<char> ComputeShaderErrorMessage(InfoLogLength + 1);
		glGetShaderInfoLog(ComputeShaderID, InfoLogLength, NULL, &ComputeShaderErrorMessage[0]);
		printf("%s\n", &ComputeShaderErrorMessage[0]);
	}

	// Link the program
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, ComputeShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if (InfoLogLength > 0) {
		vector<char> ProgramErrorMessage(InfoLogLength + 1);
		glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
		printf("%s\n", &ProgramErrorMessage[0]);
	}

	glBindProgramPipeline(progPipeline);
	glUseProgramStages(progPipeline, GL_COMPUTE_SHADER_BIT, ProgramID);
	glValidateProgramPipeline(progPipeline);
	glGetProgramPipelineiv(progPipeline, GL_VALIDATE_STATUS, &Result);

	if (Result != GL_TRUE) {
		glGetProgramPipelineiv(progPipeline, GL_INFO_LOG_LENGTH, &InfoLogLength);
		char *log = new char[InfoLogLength];
		glGetProgramPipelineInfoLog(progPipeline, InfoLogLength, 0, log);
		delete[] log;
	}

	glDetachShader(ProgramID, ComputeShaderID);
	glDeleteShader(ComputeShaderID);

	updateProg = ProgramID;
	glBindProgramPipeline(progPipeline);
	glBindProgramPipeline(0);
}

void ParticleSystem::initialize()
{
	vec4f *position = pos->map();
	for (size_t i = 0; i<size; i++) {
		position[i].x = sfrand()*0.5;
		position[i].y = sfrand()*0.5;
		position[i].z = 0.0;
		position[i].w = 1.0;
	}
	pos->unmap();

	vec4f *velocity = vel->map();
	for (size_t i = 0; i<size; i++) {
		velocity[i].x = 0.0;
		velocity[i].y = 0.0;
		velocity[i].z = 0.0;
		velocity[i].w = 0.0;
	}
	vel->unmap();
}

void ParticleSystem::update()
{
	// Invoke the compute shader to integrate the particles
	glBindProgramPipeline(progPipeline);

	glActiveTexture(GL_TEXTURE0);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, pos->getBuffer());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, vel->getBuffer());

	glDispatchCompute(GLuint(size / WORK_GROUP_SIZE), 1, 1);

	// We need to block here on compute completion to ensure that the
	// computation is done before we render
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, 0);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);

	glBindProgramPipeline(0);
}
