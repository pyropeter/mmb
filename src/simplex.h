#ifndef _MMB_SIMPLEX_H
#define _MMB_SIMPLEX_H

#include <stdint.h>

typedef struct SimplexState SimplexState;

extern double simplex2D(SimplexState *state, double x, double y);
extern SimplexState * simplexInit(uint32_t seed);
extern void simplexFree(SimplexState *state);

#endif /*_MMB_SIMPLEX_H*/