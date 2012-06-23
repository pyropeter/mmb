#include "cmwc.h"
#include "simplex.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "defs.h"

#define M_SQRT_3 1.7320508075688772935274463415058723669428052538103806L
static const double F2 = 0.5*(M_SQRT_3-1.0);
static const double G2 = (3.0-M_SQRT_3)/6.0;

typedef struct Grad {
	int x, y, z;
} Grad;

Grad grad3[12] = (Grad[]) {
	{1, 1, 0}, {-1, 1, 0}, {1, -1, 0}, {-1, -1, 0},
	{1, 0, 1}, {-1, 0, 1}, {1, 0, -1}, {-1, 0, -1},
	{0, 1, 1}, {0, -1, 1}, {0, 1, -1}, {0, -1, -1}
};

typedef struct SimplexState {
	int normal[512], mod12[512];
} SimplexState;

static int fastFloor(double x) {
	int xi = (int)x;
	if (x < xi) {
		return xi-1;
	} else {
		return xi;
	}
}

static double dot(struct Grad g, double x, double y) {
	return g.x*x + g.y*y;
}

SimplexState * simplexInit(uint32_t seed) {
	SimplexState *state = knalloc(sizeof(SimplexState));
	cmwcInit(seed);

	state->normal[0] = 0;
	int i,j;
	for (i=1 ; i <= 256 ; i++) {
		j = (int)(((double)cmwcRand() / 0x100000000) * i);
		state->normal[i] = state->normal[j];
		state->normal[j] = i;
	}

	for (i = 0 ; i < 256 ; i++) {
		state->mod12[i] = state->normal[i] % 12;
	}

	memcpy(state->normal + 256, state->normal, 256 * sizeof(int));
	memcpy(state->mod12 + 256, state->mod12, 256 * sizeof(int));

	return state;
}

void simplexFree(SimplexState *state) {
	free(state);
}

double simplex2D(SimplexState *state, double xin, double yin) {
	double n0, n1, n2;

	double s = (xin+yin)*F2;
	int i = fastFloor(xin + s);
	int j = fastFloor(yin + s);
	double t = (i + j)*G2;
	double X0 = i - t;
	double Y0 = j - t;
	double x0 = xin - X0;
	double y0 = yin - Y0;

	int i1, j1;
	if (x0 > y0) {
		i1 = 1;
		j1 = 0;
	} else {
		i1 = 0;
		j1 = 1;
	}

	double x1 = x0 - i1 + G2;
	double y1 = y0 - j1 + G2;
	double x2 = x0 - 1.0 + 2.0 * G2;
	double y2 = y0 - 1.0 + 2.0 * G2;

	int ii = i & 255;
	int jj = j & 255;
	int gi0 = state->mod12[ii+state->normal[jj]];
	int gi1 = state->mod12[ii+i1+state->normal[jj+j1]];
	int gi2 = state->mod12[ii+1+state->normal[jj+1]];

	double t0 = 0.5 - x0*x0-y0*y0;
	if(t0<0) n0 = 0.0;
	else {
		t0 *= t0;
		n0 = t0 * t0 * dot(grad3[gi0], x0, y0);
	}
	double t1 = 0.5 - x1*x1-y1*y1;
	if(t1<0) n1 = 0.0;
	else {
		t1 *= t1;
		n1 = t1 * t1 * dot(grad3[gi1], x1, y1);
	}
	double t2 = 0.5 - x2*x2-y2*y2;
	if(t2<0) n2 = 0.0;
	else {
		t2 *= t2;
		n2 = t2 * t2 * dot(grad3[gi2], x2, y2);
	}

	return 70.0 * (n0 + n1 + n2);
}
