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
	

	ofxMPMParticle *p = particles[ip];

	float *px = p->px;
	float *py = p->py;
	int pcy = p->cy;
	int pcx = p->cx;
	for (int i = 0; i<3; i++) {
		vector<ofxMPMNode*>& nrow = grid[pcx + i];
		float ppxi = px[i];
		for (int j = 0; j<3; j++) {
			ofxMPMNode *nj = nrow[pcy + j];
			phi = ppxi * py[j];
			p->u += phi * nj->ax;
			p->v += phi * nj->ay;
		}
	}

	p->v += gravity;
	//if (isMouseDragging) {
	if (ofGetMousePressed(0)) {
		float vx = abs(p->x - ofGetMouseX() / scaleFactor);
		float vy = abs(p->y - ofGetMouseY() / scaleFactor);
		float mdx = (ofGetMouseX() - ofGetPreviousMouseX()) / scaleFactor;
		float mdy = (ofGetMouseY() - ofGetPreviousMouseY()) / scaleFactor;
		if (vx < 10.0F && vy < 10.0F) {
			float weight = (1.0F - vx / 10.0F) * (1.0F - vy / 10.0F);
			p->u += weight * (mdx - p->u);
			p->v += weight * (mdy - p->v);
		}
	}

	// COLLISIONS-2
	// Plus, an opportunity to add randomness when accounting for wall collisions. 
	float xf = p->x + p->u;
	float yf = p->y + p->v;
	float wallBounceMaxRandomness = 0.03;
	if (xf < 2.0F) {
		p->u += (2.0F - xf) + ofRandom(wallBounceMaxRandomness);
	}
	else if (xf > rightEdge) {
		p->u += rightEdge - xf - ofRandom(wallBounceMaxRandomness);
	}
	if (yf < 2.0F) {
		p->v += (2.0F - yf) + ofRandom(wallBounceMaxRandomness);
	}
	else if (yf > bottomEdge) {
		p->v += bottomEdge - yf - ofRandom(wallBounceMaxRandomness);
	}


	float pu = p->u;
	float pv = p->v;
	for (int i = 0; i<3; i++) {
		vector<ofxMPMNode*>& nrow = grid[pcx + i];
		float ppxi = px[i];
		for (int j = 0; j<3; j++) {
			ofxMPMNode *nj = nrow[pcy + j];
			phi = ppxi * py[j];
			nj->u += phi * pu;
			nj->v += phi * pv;
		}
	}

	// write new values
	//if (i%2 == 1)
	particles[i].x = 0.5f;
	particles[i].y = p.y / 20;
	//nodes[i] = v;

}
