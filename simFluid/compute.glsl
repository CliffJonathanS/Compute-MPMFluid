#version 430 core
#extension GL_ARB_compute_shader : enable
#extension GL_ARB_shader_storage_buffer_object : enable



layout( std430, binding=1 ) buffer Pos {
    vec4 pos[];
};

layout( std430, binding=2 ) buffer Vel {
    vec4 vel[];
};


layout(local_size_x = 256, local_size_y = 1, local_size_z = 1) in;

vec3 sinus(vec3 p) {
	return vec3(0.001, sin(p.x*40)/500, 0.0);
}

// compute shader to update particles
void main() {
	uint i = gl_GlobalInvocationID.x;

	// thread block size may not be exact multiple of number of particles
	if (i >= 1000) return;

	// read particle position and velocity from buffers
	vec3 p = pos[i].xyz;
	vec3 v = vel[i].xyz;

	v = sinus(p);

	p += v;

	// write new values
	pos[i] = vec4(p, 1.0);
	vel[i] = vec4(v, 0.0);

}
