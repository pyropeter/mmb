/* 
 * gcc -Wall -g -c generator.c -o generator
 * 
 * */

#include <math.h>
#include "generator.h"

static char *blocks = " s";

#define TRIG_TABLE_SIZE 64
static int sinTable[TRIG_TABLE_SIZE];
static int cosTable[TRIG_TABLE_SIZE];
static int sinScale = TRIG_TABLE_SIZE / 6;
static int cosScale = TRIG_TABLE_SIZE / 8;

void generatorInit() {
	float scale = M_PI * 2 / TRIG_TABLE_SIZE;
	int i;
	for (i = 0; i < TRIG_TABLE_SIZE) {
		sinTable[i] = (int)(65536 * sin(i * scale));
		cosTable[i] = (int)(65536 * cos(i * scale));
	}
	return;
}

Block *generatorGetBlock(int x, int y, int z) {
	int height = (sinTable[x * sinScale] * cosTable[z * cosScale]) >> 24;
	
	if (height <= y)
		return blocks;
	else
		return blocks + 1;
}
