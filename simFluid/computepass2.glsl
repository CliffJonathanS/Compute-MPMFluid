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
	float *gx = p->gx;
	float *gy = p->gy;
	int   pcy = p->cy;
	int   pcx = p->cx;

	float dudx = 0.0F;
	float dudy = 0.0F;
	float dvdx = 0.0F;
	float dvdy = 0.0F;

	float gxi, pxi;
	float gxf, gyf;

	int pcxi;
	for (int i = 0; i<3; i++) {

		vector<ofxMPMNode*>& nrow = grid[pcx + i];
		gxi = gx[i];
		pxi = px[i];

		for (int j = 0; j<3; j++) {
			ofxMPMNode *nj = nrow[pcy + j];
			gxf = gxi * py[j];
			gyf = pxi * gy[j];
			dudx += nj->u * gxf;
			dudy += nj->u * gyf;
			dvdx += nj->v * gxf;
			dvdy += nj->v * gyf;
		}
	}

	float w1 = dudy - dvdx;
	float wT0 = w1 * p->T01;
	float wT1 = 0.5F * w1 * (p->T00 - p->T11);
	float D00 = dudx;
	float D01 = 0.5F * (dudy + dvdx);
	float D11 = dvdy;
	float trace = 0.5F * (D00 + D11);
	D00 -= trace;
	D11 -= trace;

	p->T00 += (-wT0 + D00) - yieldRate * p->T00;
	p->T01 += (wT1 + D01) - yieldRate * p->T01;
	p->T11 += (wT0 + D11) - yieldRate * p->T11;

	// here's our protection against exploding simulations...
	float norma = p->T00 * p->T00 + 2.0F * p->T01 * p->T01 + p->T11 * p->T11;
	if (norma > 10.0F) {
		p->T00 = p->T01 = p->T11 = 0.0F;
	}

	int cx0 = (int)p->x;
	int cy0 = (int)p->y;
	int cx1 = cx0 + 1;
	int cy1 = cy0 + 1;
	ofxMPMNode *n00 = grid[cx0][cy0];
	ofxMPMNode *n01 = grid[cx0][cy1];
	ofxMPMNode *n10 = grid[cx1][cy0];
	ofxMPMNode *n11 = grid[cx1][cy1];

	float p00 = n00->m;
	float x00 = n00->gx;
	float y00 = n00->gy;
	float p01 = n01->m;
	float x01 = n01->gx;
	float y01 = n01->gy;
	float p10 = n10->m;
	float x10 = n10->gx;
	float y10 = n10->gy;
	float p11 = n11->m;
	float x11 = n11->gx;
	float y11 = n11->gy;

	float pdx = p10 - p00;
	float pdy = p01 - p00;
	float C20 = 3.0F * pdx - x10 - 2.0F * x00;
	float C02 = 3.0F * pdy - y01 - 2.0F * y00;
	float C30 = -2.0F * pdx + x10 + x00;
	float C03 = -2.0F * pdy + y01 + y00;
	float csum1 = p00 + y00 + C02 + C03;
	float csum2 = p00 + x00 + C20 + C30;
	float C21 = 3.0F * p11 - 2.0F * x01 - x11 - 3.0F * csum1 - C20;
	float C31 = (-2.0F * p11 + x01 + x11 + 2.0F * csum1) - C30;
	float C12 = 3.0F * p11 - 2.0F * y10 - y11 - 3.0F * csum2 - C02;
	float C13 = (-2.0F * p11 + y10 + y11 + 2.0F * csum2) - C03;
	float C11 = x01 - C13 - C12 - x00;

	float u1 = p->x - (float)cx0;
	float u2 = u1 * u1;
	float u3 = u1 * u2;
	float v1 = p->y - (float)cy0;
	float v2 = v1 * v1;
	float v3 = v1 * v2;
	float density =
		p00 +
		x00 * u1 +
		y00 * v1 +
		C20 * u2 +
		C02 * v2 +
		C30 * u3 +
		C03 * v3 +
		C21 * u2 * v1 +
		C31 * u3 * v1 +
		C12 * u1 * v2 +
		C13 * u1 * v3 +
		C11 * u1 * v1;


	float DS = densitySetting;
	if (bGradient) {
		// Just for yuks, a spatially varying density function
		DS = densitySetting * (powf(p->x / (float)gridSizeX, 4.0));
	}

	float pressure = (stiffness / max(1.0F, DS)) * (density - DS);
	if (pressure > 2.0F) {
		pressure = 2.0F;
	}

	p->d = 1.0 / MAX(0.001, density);

	// COLLISIONS-1
	// Determine if there has been a collision with the wall. 
	float fx = 0.0F;
	float fy = 0.0F;
	bool bounced = false;

	if (p->x < 3.0F) {
		fx += 3.0F - p->x;
		bounced = true;
	}
	else if (p->x >(float)(gridSizeX - 3)) {
		fx += (gridSizeX - 3.0) - p->x;
		bounced = true;
	}

	if (p->y < 3.0F) {
		fy += 3.0F - p->y;
		bounced = true;
	}
	else if (p->y >(float)(gridSizeY - 3)) {
		fy += (gridSizeY - 3.0) - p->y;
		bounced = true;
	}


	// Interact with a simple demonstration obstacle.
	// Note: an accurate obstacle implementation would also need to implement
	// some velocity fiddling as in the section labeled "COLLISIONS-2" below.
	// Otherwise, this obstacle is "soft"; particles can enter it slightly. 
	if (bDoObstacles && obstacles.size() > 0) {

		// circular obstacle
		float oR = obstacles[0]->radius;
		float oR2 = obstacles[0]->radius2;
		float odx = obstacles[0]->cx - p->x;
		float ody = obstacles[0]->cy - p->y;
		float oD2 = odx*odx + ody*ody;
		if (oD2 < oR2) {
			float oD = sqrtf(oD2);
			float dR = oR - oD;
			fx -= dR * (odx / oD);
			fy -= dR * (ody / oD);
			bounced = true;
		}
	}

	trace *= stiffnessBulk;
	float T00 = elasticity * p->T00 + viscosity * D00 + pressure + trace;
	float T01 = elasticity * p->T01 + viscosity * D01;
	float T11 = elasticity * p->T11 + viscosity * D11 + pressure + trace;
	float dx, dy;

	if (bounced) {
		for (int i = 0; i<3; i++) {
			vector<ofxMPMNode*>& nrow = grid[pcx + i];
			float ppxi = px[i];
			float pgxi = gx[i];

			for (int j = 0; j<3; j++) {
				ofxMPMNode *nj = nrow[pcy + j];
				phi = ppxi * py[j];
				dx = pgxi * py[j];
				dy = ppxi * gy[j];
				nj->ax += fx * phi - (dx * T00 + dy * T01);
				nj->ay += fy * phi - (dx * T01 + dy * T11);
			}
		}

	}
	else {

		float *pppxi = &px[0];
		float *ppgxi = &gx[0];

		for (int i = 0; i<3; i++) {
			vector<ofxMPMNode*>& nrow = grid[pcx + i];

			float ppxi = *(pppxi++); //px[i]; 
			float pgxi = *(ppgxi++); //gx[i];
			for (int j = 0; j<3; j++) {
				ofxMPMNode *nj = nrow[pcy + j];
				dx = pgxi * py[j];
				dy = ppxi * gy[j];
				nj->ax -= (dx * T00 + dy * T01);
				nj->ay -= (dx * T01 + dy * T11);
			}

		}
	}

	// write new values
	//if (i%2 == 1)
	particles[i].x = 0.5f;
	particles[i].y = p.y / 20;
	//nodes[i] = v;

}
