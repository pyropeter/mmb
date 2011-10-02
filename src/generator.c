/* 
 * gcc -Wall -g -c generator.c -o generator
 * 
 * */

#include <stdio.h>
#include <math.h>
#include "generator.h"

static char *blocks = " s";

#define TRIG_TABLE_SIZE 64L
static long long int sinTable[TRIG_TABLE_SIZE];
static long long int cosTable[TRIG_TABLE_SIZE];

void generatorInit() {
	float scale = M_PI * 2 / TRIG_TABLE_SIZE;
	int i;
	for (i = 0; i < TRIG_TABLE_SIZE; i++) {
		sinTable[i] = (int)(65536 * sin(i * scale));
		cosTable[i] = (int)(65536 * cos(i * scale));
	}
	return;
}

Block *generatorGetBlock(Comp x, Comp y, Comp z) {
	Comp height = ((sinTable[(x * TRIG_TABLE_SIZE / 20) % TRIG_TABLE_SIZE]
			* cosTable[(z * TRIG_TABLE_SIZE / 30) % TRIG_TABLE_SIZE])
			/ 1000000000) + HALFCOMP;
	
	if (height <= y)
		return blocks;
	else
		return blocks + 1;
}
