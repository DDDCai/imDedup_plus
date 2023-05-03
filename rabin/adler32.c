/*
 * @Author: Cai Deng
 * @Date: 2021-06-09 03:25:31
 * @LastEditors: Cai Deng
 * @LastEditTime: 2021-06-09 03:27:07
 * @Description: 
 */
#include "adler32.h"

#define ADLER32_BASE 65521
/* NMAX is the largest n such that 255n(n+1)/2 + (n+1)(BASE-1) <= 2^32-1 */
#define ADLER32_NMAX 5552

unsigned int adler32(unsigned int adler, const void *buffer, int len) {
  const unsigned char *buf;
  unsigned int sum2, n;

  if (!buffer || (len <= 0))
    return 1;

  buf = (const unsigned char *)buffer;

  sum2 = (adler >> 16) & 0xffff;
  adler &= 0xffff;

  while (len >= ADLER32_NMAX) {
    n = ADLER32_NMAX;

    while (n--) {
      adler += *buf++;
      sum2 += adler;
    }

    adler %= ADLER32_BASE;
    sum2 %= ADLER32_BASE;

    len -= ADLER32_NMAX;
  }

  if (len > 0) {
    while (len--) {
      adler += *buf++;
      sum2 += adler;
    }

    adler %= ADLER32_BASE;
    sum2 %= ADLER32_BASE;
  }

  return adler | (sum2 << 16);
}

unsigned int adler32_combine(unsigned int adler1, unsigned int adler2,
                             int len2) {
  unsigned int sum1, sum2, rem;

  rem = (unsigned int)(len2 % ADLER32_BASE);
  sum1 = adler1 & 0xffff;
  sum2 = (rem * sum1) % ADLER32_BASE;
  sum1 += (adler2 & 0xffff) + ADLER32_BASE - 1;
  sum2 += ((adler1 >> 16) & 0xffff) + ((adler2 >> 16) & 0xffff) + ADLER32_BASE -
          rem;

  if (sum1 >= ADLER32_BASE)
    sum1 -= ADLER32_BASE;

  if (sum1 >= ADLER32_BASE)
    sum1 -= ADLER32_BASE;

  if (sum2 >= (ADLER32_BASE << 1))
    sum2 -= (ADLER32_BASE << 1);

  if (sum2 >= ADLER32_BASE)
    sum2 -= ADLER32_BASE;

  return sum1 | (sum2 << 16);
}

unsigned int adler32_rolling(unsigned int adler, unsigned char inbyte,
                             unsigned char outbyte, int window_size) {
  unsigned int sum1, sum2;

  sum1 = adler;
  sum2 = (sum1 >> 16) & 0xffff;
  sum1 &= 0xffff;

  sum1 = (sum1 - outbyte + inbyte) % ADLER32_BASE;
  sum2 = (sum2 - (window_size * outbyte) + sum1 - 1) % ADLER32_BASE;

  return (sum2 << 16) | sum1;
}