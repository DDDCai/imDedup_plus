/*
 * @Author: Cai Deng
 * @Date: 2021-06-09 03:23:04
 * @LastEditors: Cai Deng
 * @LastEditTime: 2021-06-09 03:27:36
 * @Description: 
 */

#ifndef _INCLUDE_ADLER32_H_
#define _INCLUDE_ADLER32_H_

unsigned int adler32(unsigned int adler, const void *buffer, int len);
unsigned int adler32_combine(unsigned int adler1, unsigned int adler2,
                             int len2);
unsigned int adler32_rolling(unsigned int adler, unsigned char inbyte,
                             unsigned char outbyte, int window_size);

#endif