#version 430 core
#extension GL_EXT_shader_io_blocks : enable

#UNIFORMS

//layout(location = 0) in vec3 vertexPosition_modelspace;

layout(std430, binding=1) buffer Particles {
	Particle particles[];
};

out gl_PerVertex {
    vec4 gl_Position;
};

out block {
     vec4 color;
} Out;

void main() {
    // expand points to quads without using GS
    //int particleID = gl_VertexID >> 2; // 4 vertices per particle
    vec4 particlePos = vec4(particles[gl_VertexID].x, particles[gl_VertexID].y, 0, 1.0);

    Out.color = vec4(0.4, 0.7, 1.0, 1.0);

    gl_Position = particlePos;
	//if (particlePos.x < 0)
		//gl_Position = vec4(0.0, 0.0, 0.0, 1.0);

}