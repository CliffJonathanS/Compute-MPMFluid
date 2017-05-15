#version 430 core
#extension GL_ARB_compute_shader : enable
#extension GL_ARB_shader_storage_buffer_object : enable

#UNIFORMS



layout( std430, binding=1 ) buffer Particles {
    Particle particles[];
};

layout( std430, binding=2 ) buffer Nodes {
    Node nodes[];
};


layout(local_size_x = COMPUTESIZE, local_size_y = 1, local_size_z = 1) in;

vec3 sinus(vec3 p) {
	return vec3(0.001, sin(p.x*40)/500, 0.0);
}

vec3 gravitation(vec3 p, vec3 v) {
	vec3 g = vec3(0.0, v.y - 0.003, 0.0);
	if (p.y <= -1.0) {
		g = -0.45 * g;
	}
	return g;
}

// compute shader to update particles
void main() {
	uint i = gl_GlobalInvocationID.x;

	// thread block size may not be exact multiple of number of particles
	if (i >= NPARTICLES) return;

	// read particles from buffers
	Particle p;
	Node n;
	

	ofxMPMNode *n = activeNodes[ni];
	if (n->m > 0.0F) {
		n->ax /= n->m;
		n->ay /= n->m;
		n->u = 0.0;
		n->v = 0.0;
	}

	// write new values
	//if (i%2 == 1)
	particles[i].x = 0.5f;
	particles[i].y = p.y / 20;
	//nodes[i] = v;

}
