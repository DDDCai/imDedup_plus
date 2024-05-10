/*
 * @Author: Cai Deng
 * @Date: 2020-10-12 12:50:42
 * @LastEditors: Cai Deng
 * @LastEditTime: 2022-06-21 13:01:39
 * @Description: 
 */
#ifndef _INCLUDE_JPEG_H_
#define _INCLUDE_JPEG_H_

#include "idedup.h"

#define NO_PROGRESSIVE

typedef struct 
{
    uint8_t                 *data;
    uint8_t                 *header;
    uint32_t                headerSize;
    uint32_t                imgSize[6];
    #ifdef ORIGINAL_HUFF
    JHUFF_TBL               hufTab[6];
    #endif

}   jpeg_coe, *jpeg_coe_ptr;

typedef struct 
{
    jpeg_coe_ptr    coe;
    #ifdef END_WITH_FFXX
    uint8_t         ffxx, xx;
    #endif

}   target_struct, *target_ptr;

typedef struct decodeData
{
    rawDataPtr  rawData;
    target_ptr  targetInfo;
    uint64_t    mem_size;
    struct  decodeData  *next;

}   decodedDataNode, *decodedDataPtr;

typedef struct 
{
    char    *name, *folder;
    jpeg_coe_ptr    coe;

}   de_node;

typedef struct encodedData
{
    uint8_t *data;
    uint64_t    size, mem_size;
    char    *name;
    struct encodedData *next;

}   encodedDataNode, *encodedDataPtr;

jpeg_coe_ptr get_base_coe_mem(uint8_t *data, uint32_t size);
void* decode_thread(void *parameter);
void* de_encode_and_write_thread(void *parameter);
void* de_middle_thread(void *parameter);
void* de_encode_thread(void *parameter);

#endif