/*
 * @Author: Cai Deng
 * @Date: 2020-11-19 11:28:26
 * @LastEditors: Cai Deng
 * @LastEditTime: 2021-06-21 07:57:36
 * @Description: 
 */
#ifndef _INCLUDE_2DF_H_
#define _INCLUDE_2DF_H_

#include "idedup.h"
#include "buffer.h"
#include "jpeg.h"
#include "pair.h"

extern int SF_NUM, FEA_PER_SF;
#define FEATURE_NUM (SF_NUM*FEA_PER_SF)


void* detect_thread(void *parameter);
void* tra_detect_thread(void *parameter);

#endif