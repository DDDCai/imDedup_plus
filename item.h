/*
 * @Author: Cai Deng
 * @Date: 2021-06-21 07:59:39
 * @LastEditors: Cai Deng
 * @LastEditTime: 2021-06-21 08:01:16
 * @Description: 
 */
#ifndef _INCLUDE_ITEM_H_
#define _INCLUDE_ITEM_H_

#include "idedup.h"

typedef struct digest 
{
    COPY_X x;
    COPY_Y y;
    struct  digest *next;

}   digest_node, *digest_ptr;

typedef struct 
{
    digest_ptr tail, head;
    #ifndef THREAD_OPTI
    COPY_X     lastX;
    COPY_Y     lastY;
    #endif

}   digest_list;

#endif