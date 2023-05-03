/*
 * @Author: Cai Deng
 * @Date: 2021-06-21 06:41:16
 * @LastEditors: Cai Deng
 * @LastEditTime: 2022-07-18 13:42:54
 * @Description: 
 */
#ifndef _INCLUDE_INDEX_H_
#define _INCLUDE_INDEX_H_

#include "idedup.h"
#include "buffer.h"
#include "jpeg.h"
#include "pair.h"
#include "item.h"

#define INDEX_THREAD_NUM 1  //original : 16

#define TUEN_ON_INDEX_OPTI


void* index_thread(void *parameter);

#endif