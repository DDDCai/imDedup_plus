/*
 * @Author: Cai Deng
 * @Date: 2021-06-21 07:56:44
 * @LastEditors: Cai Deng
 * @LastEditTime: 2022-05-31 08:31:12
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
    #ifdef FIX_OPTI
    uint64_t        *position;
    #endif
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
    uint64_t        fix_flag;
    struct detectionInfo    *next;

}   detectionNode, *detectionDataPtr;

#endif