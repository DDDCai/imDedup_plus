//
// Created by lenovo on 2020/1/7.
//




#include "xdelta3.h"
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#define  MBSIZE 1024*1024
char * InputFile= "/home/thl/test/test1";
char * BaseFile = "/home/thl/test/test2";

int main()
{

    uint8_t * inp = (uint8_t *)malloc(900*MBSIZE); //input
    uint8_t * bas = (uint8_t *)malloc(900*MBSIZE);
    uint8_t * del = (uint8_t *)malloc(900*MBSIZE);
    uint8_t * res = (uint8_t *)malloc(900*MBSIZE);
    int F1= open (InputFile,O_RDONLY, 0777);
    if(F1<=0){
        printf("OFile Fail:%s\n",InputFile);
        exit(0);
    }
    int F2= open (BaseFile,O_RDONLY, 0777);
    if(F2<=0){
        printf("OFile Fail:%s\n",BaseFile);
        exit(0);
    }
    usize_t Isize = 0, Bsize=0, dsize=0,rsize=0;
    int chunkLen=0;
    while( chunkLen = read(F1,(char* )(inp+Isize),MBSIZE))
    {
        Isize += chunkLen;
    }


    while(  chunkLen = read(F2,(char* )(bas+Bsize),MBSIZE))
    {
        Bsize += chunkLen;
    }

    printf("Input:%d\n",Isize);
    printf("Base:%d\n",Bsize);
    dsize =0;

    int b = xd3_encode_memory((uint8_t *) inp, Isize,
                              (uint8_t *) bas, Bsize,
                              (uint8_t *) del, (usize_t *) &dsize,
                              Isize, 1);

    printf("dsize:%d\n",dsize);

    int r2 = xd3_decode_memory( (uint8_t*)del,  (usize_t)dsize,
                                ( uint8_t*)bas, (usize_t) Bsize,
                                ( uint8_t*)res, (usize_t*) &rsize,(usize_t) Isize, 1);

    if(Isize != rsize){
        printf("restore size (%d) ERROR!\r\n", rsize);
        //exit(0);
    }

    if(memcmp(inp,res,Isize)!=0){
        printf("delta error!!!\n");
    }
    //printf("%s\n",del);

    return 0;
}