/*
 * @Author: Cai Deng
 * @Date: 2021-01-14 14:38:31
 * @LastEditors: Cai Deng
 * @LastEditTime: 2021-03-11 15:08:27
 * @Description: 
 */

#ifndef _INCLUDE_REJPEG_H_
#define _INCLUDE_REJPEG_H_

#include "idedup.h"
#include "jpeg.h"
#include "idelta.h"

#define ZIGZAG

typedef struct rejpegResult
{
    dedupResPtr dedupRes;
    uint8_t     *rejpegRes;
    uint32_t    rejpegSize;
    #ifdef COMPRESS_DELTA_INS
    uint8_t     flag;
    uint8_t     *cpx, *cpy, *cpl, *inl;
    uint32_t    cpxSize, cpySize, cplSize, inlSize;
    #endif
    uint64_t    mem_size;
    struct rejpegResult *next;

}   rejpegResNode, *rejpegResPtr;

void* rejpeg_thread(void *parameter);
void* tra_rejpeg_thread(void *parameter);
void* de_decode_thread(void *parameter);

#endif