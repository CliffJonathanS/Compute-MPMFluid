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

// compute shader to update particles
void main() {
	uint i = gl_GlobalInvocationID.x;

	// thread block size may not be exact multiple of number of particles
	if (i >= NPARTICLES) return;

	// read particles from buffers
	Particle p;
	Node n;
	p = particles[i];
	p.x *= 20;
	p.y *= 20;
	
	
	int pcx = p.cx = int(p.x - 0.5F);
	int pcy = p.cy = int(p.y - 0.5F);
	
	float[3] px = p.px;
	float[3] py = p.py;
	float[3] gx = p.gx;
	float[3] gy = p.gy;
	float pu = p.u;
	float pv = p.v;
	p.pu = pu;
	p.pv = pv;
	

	// N.B.: The constants below are not playthings.
	float x = p.cx - p.x;
	px[0] = (0.5F * x * x + 1.5F * x) + 1.125f;
	gx[0] = x + 1.5F;
	x++;
	px[1] = -x * x + 0.75F;
	gx[1] = -2.0F * x;
	x++;
	px[2] = (0.5F * x * x - 1.5F * x) + 1.125f;
	gx[2] = x - 1.5F;
	
	float y = p.cy - p.y;
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

		if ((pcxi >= -20) && (pcxi <= 20)) {
			//vector<ofxMPMNode*>& nrow = grid[pcxi]; // potential for array index out of bounds here if simulation explodes.
			float pxi = px[i];
			float gxi = gx[i];

			for (int j = 0; j<3; j++) {
				pcyj = pcy + j;

				if ((pcyj >= -20) && (pcyj <= 20)) {
					//ofxMPMNode *n = nrow[pcyj]; // potential for array index out of bounds here if simulation explodes.
					/*
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
					n->v += phi * pv;*/
				}
			}
		}
	}

	//v = sinus(p);
	//v = gravitation(p, v);

	//p += v;

	// write new values
	particles[i].x = 0.1f;
	particles[i].y = 0.5f;
	//nodes[i] = v;

}
