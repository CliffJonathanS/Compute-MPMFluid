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
	/*pos = new ShaderBuffer<vec4f>(size);
	vel = new ShaderBuffer<vec4f>(size);*/
	index = new ShaderBuffer<uint32_t>(size * 6);
	//gridWeight = new ShaderBuffer<float>(1600);
	gridIndex = new ShaderBuffer<uint32_t>(size * 6);

	nodes = new ShaderBuffer<Node>(1600);
	particles = new ShaderBuffer<Particle>(size);

	scaletowindow = 1.0;

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

	indices = gridIndex->map();
	for (size_t i = 0; i<size; i++) {
		uint32_t j = uint32_t(i << 2);
		*(indices++) = j;
		*(indices++) = j + 1;
		*(indices++) = j + 2;
		*(indices++) = j;
		*(indices++) = j + 2;
		*(indices++) = j + 3;
	}
	gridIndex->unmap();

	loadShaders();
	initialize();
	
}


ParticleSystem::~ParticleSystem()
{
	delete particles;
	delete nodes;

	glDeleteProgram(updateProg);
}

void ParticleSystem::loadShaders()
{
	if (updateProg) {
		glDeleteProgram(updateProg);
		cout << "Error ??11 : " << glGetError() << endl;
		updateProg = 0;
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
	GLuint ProgramID = glCreateShaderProgramv(GL_COMPUTE_SHADER, 1, src);
	GLint Result = GL_FALSE;
	int InfoLogLength;
	cout << "Error ?? : " << glGetError() << endl;
	// Check the program

	glGenProgramPipelines(1, &progPipeline);
	cout << "Error ?? : " << glGetError() << endl;
	
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	cout << "Error ?? : " << glGetError() << endl;
	if (InfoLogLength > 0) {
		char *log = new char[InfoLogLength];
		glGetProgramInfoLog(ProgramID, InfoLogLength, 0, log);
		cout << "Error ?? : " << glGetError() << endl;
		printf("Shader pipeline program not valid:\n%s\n", log);
		delete[] log;

	}
	

	glBindProgramPipeline(progPipeline);
	cout << "Error ?? : " << glGetError() << endl;
	glUseProgramStages(progPipeline, GL_COMPUTE_SHADER_BIT, ProgramID);
	cout << "Error ?? : " << glGetError() << endl;
	glValidateProgramPipeline(progPipeline);
	cout << "Error ?? : " << glGetError() << endl;
	glGetProgramPipelineiv(progPipeline, GL_VALIDATE_STATUS, &Result);
	cout << "Error ?? : " << glGetError() << endl;

	if (Result != GL_TRUE) {
		GLint InfoLogLength;
		glGetProgramPipelineiv(progPipeline, GL_INFO_LOG_LENGTH, &InfoLogLength);
		cout << "Error ?? : " << glGetError() << endl;
		char *log = new char[InfoLogLength];
		glGetProgramPipelineInfoLog(progPipeline, InfoLogLength, 0, log);
		cout << "Error ?? : " << glGetError() << endl;
		printf("Shader pipeline not valid:\n%s\n", log);
		delete[] log;
	}

	updateProg = ProgramID;
	//glBindProgramPipeline(progPipeline);
	cout << "Error ?? : " << glGetError() << endl;
	glBindProgramPipeline(0);
	cout << "Error ?? : " << glGetError() << endl;
}

void ParticleSystem::initialize()
{
	Particle *particle = particles->map();
	for (size_t i = 0; i<size; i++) {
		particle[i].x = sfrand()*scaletowindow;
		particle[i].y = sfrand()*scaletowindow;
		//cout << particle[i].x << endl;
	}
	particles->unmap();

	/*vec4f *velocity = vel->map();
	for (size_t i = 0; i<size; i++) {
		velocity[i].x = 0.0;
		velocity[i].y = 0.0;
		velocity[i].z = 0.0;
		velocity[i].w = 0.0;
	}
	vel->unmap();
	//scaletowindow += 0.001;

	float *weight = gridWeight->map();
	for (size_t i = 0; i<size; i++) {
		weight[i] = 0.0f;
	}
	gridWeight->unmap();*/
}

void ParticleSystem::update()
{
	// Invoke the compute shader to integrate the particles
	glBindProgramPipeline(progPipeline);

	glActiveTexture(GL_TEXTURE0);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, particles->getBuffer());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, nodes->getBuffer());
	
	glDispatchCompute(256, 1, 1);

	// We need to block here on compute completion to ensure that the
	// computation is done before we render
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, 0);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);

	glBindProgramPipeline(0);
}
