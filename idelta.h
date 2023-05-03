/*
 * @Author: Cai Deng
 * @Date: 2020-10-12 12:45:35
 * @LastEditors: Cai Deng
 * @LastEditTime: 2022-06-21 13:17:33
 * @Description: 
 */
#ifndef _INCLUDE_IDELTA_H_
#define _INCLUDE_IDELTA_H_

#include "rabin.h"
#include "gear.h"
#include "idedup.h"
#include "buffer.h"
// #include "2df.h"
#include "index.h"
#include "item.h"

#define SUB_THREAD_NUM 1    //original : 8

// #define TURN_ON_DELTA_OPTI

typedef struct dedupResult
{
    GArray      *copy_x, *copy_y, *copy_l, *insert_l;
    GArray      *insert_p;
    uint8_t     *header;
    uint32_t    headerSize, y_counter, u_counter, v_counter;
    #ifdef JPEG_SEPA_COMP
    uint32_t    p_counter[3];
    #endif
    char        *baseName, *name;
    uint32_t    imgSize[4];
    uint8_t     ffxx, xx;
    buf_node    *node;
    uint64_t    mem_size;
    #ifdef ORIGINAL_HUFF
    JHUFF_TBL   hufTab[6];
    #endif
    struct dedupResult  *next;

}   dedupResNode, *dedupResPtr;

typedef struct de_dedupData
{
    char    *name;
    jvirt_barray_ptr *coe;
    jpeg_coe_ptr    content;
    uint8_t     *oriPtr;
    uint8_t ffxx, xx;
    uint32_t mem_size;
    struct  de_dedupData    *next;

}   de_dedupNode, *de_dedupPtr;

GHashTable **create_block_index(jpeg_coe_ptr base, uint64_t *size);
void* dedup_thread(void *parameter);
void* tra_dedup_thread(void *parameter);
void* de_dedup_thread(void *parameter);

#endif