/*
 * @Author: Cai Deng
 * @Date: 2021-06-21 07:56:44
 * @LastEditors: Cai Deng
 * @LastEditTime: 2022-05-20 02:22:16
 * @Description: 
 */
#ifndef _INCLUDE_PAIR_H_
#define _INCLUDE_PAIR_H_

#include "idedup.h"
#include "jpeg.h"
#include "buffer.h"

typedef struct 
{
    decodedDataPtr  decdData;
    uint64_t        *sfs;
    #ifdef FEATURE_CHECK
    short           *dc;
    #endif
    
}   imageData, *imagePtr;

typedef struct detectionInfo
{
    buf_node        *base, *target;
    #ifdef THREAD_OPTI
    GHashTable      **subBlockTab;
    #endif
    uint64_t        mem_size;
    struct detectionInfo    *next;

}   detectionNode, *detectionDataPtr;

#endif