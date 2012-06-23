#ifndef _MMB_CMWC_H
#define _MMB_CMWC_H

#include <stdint.h>

extern void cmwcInit(uint32_t seed);
extern uint32_t cmwcRand(void);

#endif /*_MMB_CMWC_H*/