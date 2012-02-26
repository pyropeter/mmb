/* 
 * gcc -Wall -lm -g -c generator.c -o generator
 * 
 * */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "defs.h"
#include "vector.h"
#include "generator.h"

static char *blocks = " s";

#define TABLELEN 64L
static long long int sinTable[TABLELEN];
static long long int cosTable[TABLELEN];

void generatorInit() {
	float scale = M_PI * 2 / TABLELEN;
	int i;
	for (i = 0; i < TABLELEN; i++) {
		sinTable[i] = (int)(65536 * sin(i * scale));
		cosTable[i] = (int)(65536 * cos(i * scale));
	}
	return;
}

Block *generatorGetBlock(Vector3i pos) {
	int height = ((sinTable[modPositive(
				divRoundDown(pos.x * TABLELEN, 20), TABLELEN)]
			* cosTable[modPositive(
				divRoundDown(pos.z * TABLELEN, 30), TABLELEN)])
			/ 1500000000);

	if (height <= pos.y)
		return blocks;
	else
		return blocks + 1;
}
