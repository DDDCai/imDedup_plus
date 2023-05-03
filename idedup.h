/*
 * @Author: Cai Deng
 * @Date: 2020-11-09 14:22:29
 * @LastEditors: Cai Deng
 * @LastEditTime: 2022-07-19 06:21:08
 * @Description: 
 */
#ifndef _INCLUDE_IDEDUP_H_
#define _INCLUDE_IDEDUP_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <pthread.h>
#include "fse/lib/fse.h"
#include "fse/lib/huf.h"
#include "glib.h"
#include "stddef.h"
#include "jpeglib.h"
#include <unistd.h>

/*------------------------------------------*/

#define DEBUG_1
// #define CHECK_BUFFER
// #define CHECK_DECOMPRESS
// #define DO_NOT_WRITE
#define PART_TIME
// #define COLLISION_RATE

#define DEBUG_2

/*------------------------------------------*/

// #define HEADER_DELTA
#define COMPRESS_DELTA_INS
#define THREAD_OPTI
#define JPEG_SEPA_COMP
// #define OMIT_LOW_DELTA
// #define FEATURE_CHECK
// #define UNIFORMLY_SAMPLE
// #define DC_HASH

/*------------------------------------------*/

#define _RABIN 0
#define _GEAR 1
#define _2DF 2

#define USE_RABIN
#define USE_GEAR
#ifdef USE_RABIN
#undef USE_GEAR
#endif

#define RAW 0
#define DECODED 1

/*------------------------------------------*/

extern int READ_THREAD_NUM;
extern int DECODE_THREAD_NUM;
extern int MIDDLE_THREAD_NUM;
extern int REJPEG_THREAD_NUM;
extern int WRITE_THREAD_NUM;

/*------------------------------------------*/

extern int64_t DECODE_BUFFER_SIZE;  /* if it represents the image 
/* number, it should be bigger than MIDDLE_THREAD_NUM; or if 
/* it is the absolute space size, it should be bigger than the 
/* size of MIDDLE_THREAD_NUM pieces of images.  */
#define START_TO_MOVE (DECODE_BUFFER_SIZE>>1) /* if it represents the image 
/* number, it should be bigger than 1; or if it is the absolute 
/* space size, it should be bigger than the size of one piece 
/* of image.  */

/*------------------------------------------*/

#define END_WITH_FFXX

/*------------------------------------------*/

#define MAX_PATH_LEN 256

extern int64_t NAME_LIST_MAX;
extern int64_t READ_LIST_MAX;
extern int64_t DECD_LIST_MAX;
extern int64_t DECT_LIST_MAX;
extern int64_t INDX_LIST_MAX;
extern int64_t DEUP_LIST_MAX;
extern int64_t REJG_LIST_MAX;

/*------------------------------------------*/

extern int64_t PATCH_SIZE;

/*------------------------------------------*/

#define HUFFMAN
#define ORIGINAL_HUFF
#ifdef ORIGINAL_HUFF
#define HUFFMAN
typedef struct HUF_CElt_s_ {
  uint16_t  val;
  uint8_t nbBits;
} HUF_FSE; 
#endif

/*------------------------------------------*/

#define S_SHORT_UNITY   int8_t
#define U_SHORT_UNITY   uint8_t
#define S_MEDIUM_UNITY  int16_t
#define U_MEDIUM_UNITY  uint16_t
#define S_LONG_UNITY    int32_t
#define U_LONG_UNITY    uint32_t

#define S_UNITY S_MEDIUM_UNITY
#define U_UNITY U_MEDIUM_UNITY

#define COPY_X U_UNITY  //  X.
#define COPY_Y U_UNITY  //  Y.
#define COPY_L U_UNITY  //  L.

#define INSERT_L U_UNITY

/*------------------------------------------*/

#define PUT_3_STRS_TOGETHER(des,src1,src2,src3) \
{                                               \
    strcpy(des,src1);                           \
    strcat(des,src2);                           \
    strcat(des,src3);                           \
}

typedef struct 
{
    void            *head, *tail;
    pthread_mutex_t mutex;
    pthread_cond_t  rCond, wCond;             /* read & write. */
    uint64_t        size;
    u_int32_t       counter;
    u_int8_t        ending;

}   ListNode, *List;

#define INIT_LIST(list, oriSize)              \
{                                             \
    (list) = (List)malloc(sizeof(ListNode));  \
    (list)->head  =   NULL;                   \
    (list)->tail  =   NULL;                   \
    (list)->size  =   (oriSize);              \
    (list)->counter   =   0;                  \
    pthread_mutex_init(&(list)->mutex,NULL);  \
    pthread_cond_init(&(list)->rCond,NULL);   \
    pthread_cond_init(&(list)->wCond,NULL);   \
    (list)->ending    =   0;                  \
}

#define DESTROY_LIST(list)                    \
{                                             \
    pthread_mutex_destroy(&(list)->mutex);    \
    pthread_cond_destroy(&(list)->rCond);     \
    pthread_cond_destroy(&(list)->wCond);     \
    free(list);                               \
}

/*------------------------------------------*/

typedef struct dirData
{
    char     second_dir[MAX_PATH_LEN];
    uint64_t mem_size;
    struct   dirData *next;

}   dirDataNode, *dirDataPtr;

typedef struct nameData
{
    char     second_dir[MAX_PATH_LEN], file_name[MAX_PATH_LEN];
    uint64_t mem_size;
    uint8_t  end_of_dir;
    struct   nameData    *next;

}   nameDataNode, *nameDataPtr;

typedef struct rawData 
{
    char        *name, *dir_name;
    u_int8_t    *data;
    u_int32_t   size, mem_size;
    struct rawData  *next;

}   rawDataNode, *rawDataPtr;

/*------------------------------------------*/

uint32_t entropy_compress(void *src, uint32_t srcSize, void *dst, uint32_t dstSize);
#ifdef ORIGINAL_HUFF
uint64_t HUF_compress_JPEGtab(void *src, uint32_t srcSize, void *dst, uint32_t dstSize, HUF_FSE *hufTab);
#endif
uint64_t* idedup_compress(char *inFolder, char *outFolder);

/*------------------------------------------*/

typedef struct de_readData
{
    char        *name, *basename_and_oriptr;
    uint32_t    *sizes, mem_size;
    uint8_t     *header, *x, *y, *cp_l, *in_l, *in_d;
    uint8_t     ffxx, xx;
    #ifdef  COMPRESS_DELTA_INS
    uint8_t     flag;
    #endif
    struct  de_readData *next;

}   de_readNode, *de_readPtr;

/*------------------------------------------*/

// uint64_t idedup_decompress(char *inFolder, char *outFolder
//     #ifdef  CHECK_DECOMPRESS
//         , char *oriFolder
//     #endif
// );
uint64_t* idedup_decompress(char *inFolder, char *outFolder, char *idpPath
    #ifdef  CHECK_DECOMPRESS
    , char *oriFolder
    #endif
);

/*------------------------------------------*/

#endif