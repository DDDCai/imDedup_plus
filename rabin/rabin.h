/*
 * @Author: Cai Deng
 * @Date: 2021-01-18 19:48:07
 * @LastEditors: Cai Deng
 * @LastEditTime: 2021-04-23 03:29:45
 * @Description: 
 */
#ifndef _RABIN_H
#define _RABIN_H

#include <stdint.h>

#define LBS 1   //  Logic Block Size.
#define LBS2 (LBS*LBS)

#define POLYNOMIAL 0x3DA3358B4DC173LL
#define POLYNOMIAL_DEGREE 53
// #define WINSIZE 64
// #define WINSIZE (LBS2<<7)
#define AVERAGE_BITS 20

struct rabin_t {
    uint8_t *window;
    unsigned int wpos;
    unsigned int count;
    unsigned int pos;
    unsigned int start;
    uint64_t digest;
};

struct chunk_t {
    unsigned int start;
    unsigned int length;
    uint64_t cut_fingerprint;
};

extern struct chunk_t last_chunk;

struct rabin_t *rabin_init(uint64_t winsize, uint64_t *out_table, uint64_t *mod_table);
void rabin_reset(struct rabin_t *h, uint64_t winsize, uint64_t *out_table, uint64_t *mod_table);
void rabin_slide(struct rabin_t *h, uint8_t b, uint64_t winsize, uint64_t *out_table, uint64_t *mod_table);
void rabin_append(struct rabin_t *h, uint8_t b, uint64_t *mod_table);
void rabin_slide_a_block(struct rabin_t *h, uint8_t *block_p, uint64_t winsize, uint64_t *out_table, uint64_t *mod_table);
void rabin_free(struct rabin_t *h);
void calc_tables(uint64_t WINSIZE, uint64_t *out_table, uint64_t *mod_table);

#endif
