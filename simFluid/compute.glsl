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


layout(local_size_x = 256, local_size_y = 1, local_size_z = 1) in;

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
	if (i >= 1000) return;

	// read particles from buffers
	Particle p;
	Node n;
	float x = particles[i].x;
	float y = particles[i].y;
	p.x = 1.0f;
	p.y = 0.5f;
	p.u = p.v = p.pu = p.pv = p.d = p.gu = p.gv = p.T00 = p.T01 = p.T11 = 0;
	p.cx = p.cy = 0;
	p.px[0] = p.px[1] = p.px[2] = p.py[0] = p.py[1] = p.py[2] = p.gx[0] = p.gx[1] = p.gx[2] = p.gy[0] = p.gy[1] = p.gy[2] = 0;

	//v = sinus(p);
	//v = gravitation(p, v);

	/*int cx = (int) pos[i].x - 0.5f;
	int cy = (int) pos[i].y - 0.5f;

	float x = (float)cx - pos[i].x;
	px[0] = (0.5F * x * x + 1.5F * x) + 1.125f;
	gx[0] = x + 1.5F;
	x++;
	px[1] = -x * x + 0.75F;
	gx[1] = -2.0F * x;
	x++;
	px[2] = (0.5F * x * x - 1.5F * x) + 1.125f;
	gx[2] = x - 1.5F;

	float y = (float)cy - pos[i].y;
	py[0] = (0.5F * y * y + 1.5F * y) + 1.125f;
	gy[0] = y + 1.5F;
	y++;
	py[1] = -y * y + 0.75F;
	gy[1] = -2.0F * y;
	y++;
	py[2] = (0.5F * y * y - 1.5F * y) + 1.125f;
	gy[2] = y - 1.5F;


	int pcxi, pcyj;
	for (int i = 0; i<3; i++) {
		pcxi = pcx + i;

		if ((pcxi >= 0) && (pcxi < gridSizeX)) {
			vector<ofxMPMNode*>& nrow = grid[pcxi]; 
			float pxi = px[i];
			float gxi = gx[i];

			for (int j = 0; j<3; j++) {
				pcyj = pcy + j;

				if ((pcyj >= 0) && (pcyj < gridSizeY)) {
					ofxMPMNode *n = nrow[pcyj]; 

					if (!n->active) {
						n->active = true;
						activeNodes[numActiveNodes] = n;
						numActiveNodes++;
					}
					phi = pxi * py[j];
					n->m += phi;
					n->gx += gxi * py[j];
					n->gy += pxi * gy[j];
					n->u += phi * pu;
					n->v += phi * pv;
				}
			}
		}
	}*/

	//p += v;

	// write new values
	particles[i].x = x;
	particles[i].y = y;
	//nodes[i] = v;

}
