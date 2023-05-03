#ifndef _INCLUDE_GEAR_H_
#define _INCLUDE_GEAR_H_

#include <stdint.h>

// void gear_slide_a_block(uint64_t *h, uint8_t *block_p);
uint64_t gear_slide_a_block(uint8_t *ptr);
uint64_t gear_slide(uint64_t h, uint8_t thisByte);

#endif