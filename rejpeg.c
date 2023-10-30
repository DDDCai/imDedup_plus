/*
 * @Author: Cai Deng
 * @Date: 2021-01-14 14:38:26
 * @LastEditors: Cai Deng
 * @LastEditTime: 2022-07-18 14:34:40
 * @Description: 
 */
// #include "rejpeg.h"

// #ifdef PART_TIME
// extern double rejpeg_time;
// extern pthread_mutex_t rejpeg_time_mutex;
// #endif

// typedef struct 
// {
//     short       *value;
//     u_int8_t    *length;
//     u_int32_t   valLen, lenLen;

// }   rleResult;

// #ifdef ZIGZAG
// char zz_sequence[64] = 
// {
//     0 ,1 ,5 ,6 ,14,15,27,28,
//     2 ,4 ,7 ,13,16,26,29,42,
//     3 ,8 ,12,17,25,30,41,43,
//     9 ,11,18,24,31,40,44,53,
//     10,19,23,32,39,45,52,54,
//     20,22,33,38,46,51,55,60,
//     21,34,37,47,50,56,59,61,
//     35,36,48,49,57,58,62,63,
// };

// static short* zigzag(short *ptr)
// {
//     short   *tmp =  (short*)malloc(sizeof(short)<<6);
//     int     i;

//     for(i=0;i<64;i++)
//     {
//         tmp[zz_sequence[i]] =   ptr[i];
//     }

//     return tmp;
// }
// #endif  // #ifdef ZIGZAG

// // static rleResult rleEncode(GPtrArray *dataPtr
// static rleResult rleEncode(GArray *dataPtr
//     #ifdef JPEG_SEPA_COMP
//     , uint32_t from, uint32_t componentLen
//     #endif
// )
// {
//     u_int32_t   valLen  = 0, lenLen = 0;
//     #ifdef JPEG_SEPA_COMP
//     u_int8_t    *length = (u_int8_t*)malloc(componentLen << 6);
//     short       *value  = (short*)malloc(sizeof(short)*componentLen << 6);
//     #else
//     u_int8_t    *length = (u_int8_t*)malloc(dataPtr->len << 6);
//     short       *value  = (short*)malloc(sizeof(short)*dataPtr->len << 6);
//     #endif
//     u_int32_t   zeroCounter  = 0;
//     int         i, j, h;
//     short       *ptr;
//     short       lastDC  = 0, thisDC;
//     rleResult   result;

//     #ifndef JPEG_SEPA_COMP
//     for(i=0;i<dataPtr->len;i++)
//     #else
//     for(i=0;i<componentLen;i++)
//     #endif
//     {
//         #ifndef JPEG_SEPA_COMP
//         // ptr = (short*)g_ptr_array_index(dataPtr,i);
//         ptr = (short*)g_array_index(dataPtr, uint8_t*, i);
//         #else
//         // ptr = (short*)g_ptr_array_index(dataPtr,i+from);
//         ptr = (short*)g_array_index(dataPtr,uint8_t*,i+from);
//         #endif
//         #ifdef  ZIGZAG
//         ptr = zigzag(ptr);
//         #endif
//         thisDC = ptr[0];
//         ptr[0] = thisDC - lastDC;
//         lastDC = thisDC;

//         for(h=63;h>=0;h--)
//             if(ptr[h]) break;

//         for(j=0;j<=h;j++)
//         {
//             if(ptr[j])
//             {
//                 value[valLen++] = ptr[j];
//                 length[lenLen++] = zeroCounter;
//                 zeroCounter = 0;
//             }
//             else 
//             {
//                 if(zeroCounter == 15)
//                 {
//                     value[valLen++] = 0;
//                     length[lenLen++] = 15;
//                     zeroCounter = 0;
//                 }
//                 else 
//                     zeroCounter ++;
//             }
//         }
//         if(h<63)
//         {
//             value[valLen++] = 0;
//             length[lenLen++] = 0;
//         }
        
//         #ifdef  ZIGZAG
//         free(ptr);
//         #else
//         ptr[0]  =   lastDC;
//         #endif
//     }

//     result.length = length;
//     result.value  = value;
//     result.lenLen = lenLen;
//     result.valLen = valLen;

//     return result;
// }

// static void vliEncode(rleResult *rle, uint8_t *buf)
// {
//     u_int8_t    *lenPtr = rle->length;
//     short       *valPtr = rle->value;
//     u_int16_t   *data   = (u_int16_t*)buf;
//     short       number;
//     u_int16_t   _number_;
//     u_int32_t   size    = rle->valLen;
//     u_int8_t    bitLength; 
//     u_int16_t   bits;
//     int         dataPos = 0, emptyBits = 16, i;

//     for(i=0;i<size;i++)
//     {
//         number   = valPtr[i];
//         _number_ = abs(number);
//         if     (_number_==0   )   bitLength = 0;
//         else if(_number_==1   )   bitLength = 1;
//         else if(_number_<=3   )   bitLength = 2;
//         else if(_number_<=7   )   bitLength = 3;
//         else if(_number_<=15  )   bitLength = 4;
//         else if(_number_<=31  )   bitLength = 5;
//         else if(_number_<=63  )   bitLength = 6;
//         else if(_number_<=127 )   bitLength = 7;
//         else if(_number_<=255 )   bitLength = 8;
//         else if(_number_<=511 )   bitLength = 9;
//         else if(_number_<=1023)   bitLength = 10;
//         else if(_number_<=2047)   bitLength = 11;
//         else if(_number_<=4095)   bitLength = 12;

//         lenPtr[i] = (lenPtr[i] << 4) | bitLength;

//         if(number)
//         {
//             if(number > 0)  bits = number;
//             else bits = (-number) ^ (0xffff >> (16 - bitLength));

//             if(emptyBits >= bitLength)
//             {
//                 emptyBits -= bitLength;
//                 data[dataPos] |= bits << emptyBits;
//             }
//             else 
//             {
//                 bitLength -= emptyBits;
//                 data[dataPos] |= bits >> bitLength;
//                 emptyBits = 16 - bitLength;
//                 data[++dataPos] |= bits << emptyBits;
//             }
//         }
//     }

//     free(rle->value);
//     rle->value = data;
//     if(size)
//         rle->valLen = dataPos + 1;
// }

// /**
//  * [4 bytes] (the size of value data : n);
//  * [n bytes] (value data);
//  * [1 byte ] (if length data has been compressed);
//  * [4 bytes] (the size of length data : m);
//  * [m bytes] (length data).
//  * */
// // static int recompressed_by_myjpeg(GPtrArray *dataPtr, uint8_t *buf, uint32_t bufSize
// static int recompressed_by_myjpeg(GArray *dataPtr, uint8_t *buf, uint32_t bufSize
//     #ifdef JPEG_SEPA_COMP
//     , uint32_t from, uint32_t compoentLen
//     #endif
// )
// {
//     uint32_t size   =   sizeof(uint32_t);
//     uint8_t *ptr    =   buf + size;
//     rleResult rle   =   rleEncode(dataPtr
//         #ifdef JPEG_SEPA_COMP
//         ,from,compoentLen
//         #endif
//         );
//     vliEncode(&rle,ptr);

//     uint32_t valSize=   rle.valLen*sizeof(uint16_t);
//     memcpy(buf, &valSize, sizeof(valSize));
//     ptr += valSize;
//     size+= valSize;

//     size    +=  (sizeof(uint8_t) + sizeof(uint32_t));
//     uint8_t *pre = ptr;
//     ptr += (sizeof(uint8_t) + sizeof(uint32_t));
//     uint32_t lenLen = entropy_compress(rle.length, rle.lenLen, ptr, bufSize - size);
//     if(lenLen > 0)
//         *pre++ = 1;
//     else 
//     {
//         if(rle.lenLen > (bufSize - size))
//         {
//             free(rle.length);
//             return -1;
//         }
//         *pre++ = 0;
//         memcpy(ptr, rle.length, rle.lenLen);
//         lenLen = rle.lenLen;
//     }
//     ptr  += lenLen;
//     size += lenLen;
//     memcpy(pre, &lenLen, sizeof(lenLen));

//     free(rle.length);

//     return size;
// }

// static rejpegResPtr rejpeg_a_single_img(dedupResPtr dedupPtr)
// {
//     uint32_t    bfSize  =   dedupPtr->insert_p->len << 7;
//     uint8_t     *buf    =   NULL;
//     int         size    =   0;
//     rejpegResPtr    rejpegPtr   =   (rejpegResPtr)malloc(sizeof(rejpegResNode));

//     for( ; bfSize>0; bfSize <<= 1)
//     {
//         #ifndef JPEG_SEPA_COMP
//         buf     =   (uint8_t*)g_malloc0(bfSize);
//         size    =   recompressed_by_myjpeg(dedupPtr->insert_p, buf, bfSize);
//         if(size < 0)
//             free(buf);
//         else 
//             break;
//         #else
//         size = 0;
//         int sizeTmp;
//         buf     =   (uint8_t*)g_malloc0(bfSize);
//         sizeTmp =   recompressed_by_myjpeg(dedupPtr->insert_p, buf, bfSize, 0, dedupPtr->p_counter[0]);
//         if(sizeTmp < 0)
//         {
//             free(buf);
//             continue;
//         }
//         size += sizeTmp;
//         sizeTmp =   recompressed_by_myjpeg(dedupPtr->insert_p, buf+size, bfSize-size, dedupPtr->p_counter[0], dedupPtr->p_counter[1]);
//         if(sizeTmp < 0)
//         {
//             free(buf);
//             continue;
//         }
//         size += sizeTmp;
//         sizeTmp =   recompressed_by_myjpeg(dedupPtr->insert_p, buf+size, bfSize-size, dedupPtr->p_counter[0]+dedupPtr->p_counter[1], dedupPtr->p_counter[2]);
//         if(sizeTmp < 0)
//         {
//             free(buf);
//             continue;
//         }
//         size += sizeTmp;
//         break;
//         #endif
//     }

//     rejpegPtr->dedupRes     =   dedupPtr;
//     rejpegPtr->rejpegRes    =   buf;
//     rejpegPtr->rejpegSize   =   size;
//     rejpegPtr->mem_size     =   size + sizeof(rejpegResNode) + dedupPtr->mem_size;

//     return  rejpegPtr;
// }

// #ifdef COMPRESS_DELTA_INS
// static void compress_delta_ins(rejpegResPtr rejpeg)
// {
//     uint32_t    cpxSize =   rejpeg->dedupRes->copy_x->len*sizeof(COPY_X),
//                 cpySize =   rejpeg->dedupRes->copy_y->len*sizeof(COPY_Y),
//                 cplSize =   rejpeg->dedupRes->copy_l->len*sizeof(COPY_L),
//                 inlSize =   rejpeg->dedupRes->insert_l->len*sizeof(INSERT_L);
//     rejpeg->cpx =   (uint8_t*)malloc(cpxSize);
//     rejpeg->cpy =   (uint8_t*)malloc(cpySize);
//     rejpeg->cpl =   (uint8_t*)malloc(cplSize);
//     rejpeg->inl =   (uint8_t*)malloc(inlSize);

//     rejpeg->cpxSize =   entropy_compress(rejpeg->dedupRes->copy_x->data, cpxSize, rejpeg->cpx, cpxSize);
//     if(rejpeg->cpxSize > 0)
//         rejpeg->flag    =   0x08;
//     else 
//     {
//         rejpeg->flag    =   0;
//         free(rejpeg->cpx);
//         rejpeg->cpx     =   NULL;
//         rejpeg->cpxSize =   cpxSize;
//     }
//     rejpeg->cpySize =   entropy_compress(rejpeg->dedupRes->copy_y->data, cpySize, rejpeg->cpy, cpySize);
//     if(rejpeg->cpySize > 0)
//         rejpeg->flag    |=  0x04;
//     else 
//     {
//         free(rejpeg->cpy);
//         rejpeg->cpy     =   NULL;
//         rejpeg->cpySize =   cpySize;
//     }
//     rejpeg->cplSize =   entropy_compress(rejpeg->dedupRes->copy_l->data, cplSize, rejpeg->cpl, cplSize);
//     if(rejpeg->cplSize > 0)
//         rejpeg->flag    |=  0x02;
//     else 
//     {
//         free(rejpeg->cpl);
//         rejpeg->cpl     =   NULL;
//         rejpeg->cplSize =   cplSize;
//     }
//     rejpeg->inlSize =   entropy_compress(rejpeg->dedupRes->insert_l->data, inlSize, rejpeg->inl, inlSize);
//     if(rejpeg->inlSize > 0)
//         rejpeg->flag    |=  0x01;
//     else 
//     {
//         free(rejpeg->inl);
//         rejpeg->inl     =   NULL;
//         rejpeg->inlSize =   inlSize;
//     }
// }
// #endif

// void* rejpeg_thread(void *parameter)
// {
//     void        **arg       =   (void**)parameter;
//     List        dedupList   =   (List)arg[0];
//     List        rejpegList  =   (List)arg[1];
//     char        *outPath    =   (char*)arg[2];
//     dedupResPtr dedupPtr;
//     rejpegResPtr    rejpegPtr;
//     #ifdef PART_TIME
//     GTimer      *timer      =   g_timer_new();
//     #endif

//     while(1)
//     {
//         pthread_mutex_lock(&dedupList->mutex);
//         while(dedupList->counter == 0)
//         {
//             if(dedupList->ending == MIDDLE_THREAD_NUM) goto ESCAPE_LOOP;
//             pthread_cond_wait(&dedupList->rCond, &dedupList->mutex);
//         }
//         dedupPtr = dedupList->head;
//         dedupList->head =   dedupPtr->next;
//         dedupList->counter --;
//         dedupList->size +=  dedupPtr->mem_size;
//         for(int i=0; i<MIDDLE_THREAD_NUM; i++)
//         pthread_cond_signal(&dedupList->wCond);
//         pthread_mutex_unlock(&dedupList->mutex);

//         if(dedupPtr)
//         {
//             #ifdef PART_TIME
//             g_timer_start(timer);
//             #endif

//             rejpegPtr   =   rejpeg_a_single_img(dedupPtr);
//             pthread_mutex_lock(&dedupPtr->node->mutex);
//             dedupPtr->node->link --;
//             pthread_mutex_unlock(&dedupPtr->node->mutex);
//             #ifdef COMPRESS_DELTA_INS
//             compress_delta_ins(rejpegPtr);
//             rejpegPtr->mem_size +=  (rejpegPtr->cpxSize + rejpegPtr->cpySize + rejpegPtr->cplSize + rejpegPtr->inlSize);
//             #endif

//             #ifdef PART_TIME
//             pthread_mutex_lock(&rejpeg_time_mutex);
//             rejpeg_time +=  g_timer_elapsed(timer, NULL);
//             pthread_mutex_unlock(&rejpeg_time_mutex);
//             #endif

//             rejpegPtr->next =   NULL;
//             pthread_mutex_lock(&rejpegList->mutex);
//             while(rejpegList->size < rejpegPtr->mem_size)
//                 pthread_cond_wait(&rejpegList->wCond, &rejpegList->mutex);
//             if(rejpegList->counter)
//                 ((rejpegResPtr)rejpegList->tail)->next  =   rejpegPtr;
//             else 
//                 rejpegList->head    =   rejpegPtr;
//             rejpegList->tail    =   rejpegPtr;
//             rejpegList->counter ++;
//             rejpegList->size    -=  rejpegPtr->mem_size;
//             pthread_cond_signal(&rejpegList->rCond);
//             pthread_mutex_unlock(&rejpegList->mutex);
//         }
//     }

//     ESCAPE_LOOP:
//     pthread_mutex_unlock(&dedupList->mutex);

//     #ifdef PART_TIME
//     g_timer_destroy(timer);
//     #endif

//     pthread_mutex_lock(&rejpegList->mutex);
//     rejpegList->ending  ++;
//     for(int i=0; i<WRITE_THREAD_NUM; i++)
//         pthread_cond_signal(&rejpegList->rCond);
//     pthread_mutex_unlock(&rejpegList->mutex);
// }

// void* tra_rejpeg_thread(void *parameter)
// {
//     void        **arg       =   (void**)parameter;
//     List        dedupList   =   (List)arg[0];
//     List        rejpegList  =   (List)arg[1];
//     char        *outPath    =   (char*)arg[2];
//     dedupResPtr dedupPtr;
//     rejpegResPtr    rejpegPtr;
//     #ifdef PART_TIME
//     GTimer      *timer      =   g_timer_new();
//     #endif

//     while(1)
//     {
//         pthread_mutex_lock(&dedupList->mutex);
//         while(dedupList->counter == 0)
//         {
//             if(dedupList->ending) goto ESCAPE_LOOP;
//             pthread_cond_wait(&dedupList->rCond, &dedupList->mutex);
//         }
//         dedupPtr = dedupList->head;
//         dedupList->head =   dedupPtr->next;
//         dedupList->counter --;
//         dedupList->size +=  dedupPtr->mem_size;
//         pthread_cond_signal(&dedupList->wCond);
//         pthread_mutex_unlock(&dedupList->mutex);

//         if(dedupPtr)
//         {
//             #ifdef PART_TIME
//             g_timer_start(timer);
//             #endif

//             rejpegPtr   =   (rejpegResPtr)malloc(sizeof(rejpegResNode));
//             rejpegPtr->dedupRes     =   dedupPtr;
//             rejpegPtr->rejpegRes    =   NULL;
//             rejpegPtr->rejpegSize   =   0;
//             rejpegPtr->mem_size     =   sizeof(rejpegResNode) + dedupPtr->mem_size;
//             uint8_t     *fseBuffer = (uint8_t*)malloc(dedupPtr->v_counter);
//             dedupPtr->y_counter =   entropy_compress(
//                 dedupPtr->insert_l,dedupPtr->v_counter,fseBuffer,dedupPtr->v_counter);
//             if(dedupPtr->y_counter > 0)
//             {
//                 free(dedupPtr->insert_l);
//                 dedupPtr->v_counter = dedupPtr->y_counter;
//                 dedupPtr->insert_l = (GArray*)fseBuffer;
//             }
//             else    free(fseBuffer);

//             pthread_mutex_lock(&dedupPtr->node->mutex);
//             dedupPtr->node->link --;
//             pthread_mutex_unlock(&dedupPtr->node->mutex);

//             #ifdef PART_TIME
//             pthread_mutex_lock(&rejpeg_time_mutex);
//             rejpeg_time +=  g_timer_elapsed(timer, NULL);
//             pthread_mutex_unlock(&rejpeg_time_mutex);
//             #endif

//             rejpegPtr->next =   NULL;
//             pthread_mutex_lock(&rejpegList->mutex);
//             while(rejpegList->size < rejpegPtr->mem_size)
//                 pthread_cond_wait(&rejpegList->wCond, &rejpegList->mutex);
//             if(rejpegList->counter)
//                 ((rejpegResPtr)rejpegList->tail)->next  =   rejpegPtr;
//             else 
//                 rejpegList->head    =   rejpegPtr;
//             rejpegList->tail    =   rejpegPtr;
//             rejpegList->counter ++;
//             rejpegList->size    -=  rejpegPtr->mem_size;
//             pthread_cond_signal(&rejpegList->rCond);
//             pthread_mutex_unlock(&rejpegList->mutex);
//         }
//     }

//     ESCAPE_LOOP:
//     pthread_mutex_unlock(&dedupList->mutex);

//     #ifdef PART_TIME
//     g_timer_destroy(timer);
//     #endif

//     pthread_mutex_lock(&rejpegList->mutex);
//     rejpegList->ending  ++;
//     for(int i=0; i<WRITE_THREAD_NUM; i++)
//         pthread_cond_signal(&rejpegList->rCond);
//     pthread_mutex_unlock(&rejpegList->mutex);
// }

// /*-------------------------------------------------------------------------*/

// static void vliDecode(rleResult *rle)
// {
//     u_int32_t   sum        = rle->lenLen;
//     u_int8_t    lengthTmp, length;
//     u_int8_t    *lengthPtr = rle->length;
//     u_int16_t   *dataPtr   = rle->value;
//     short       *buf       = (short*)malloc(sizeof(short)*sum);
//     int         fullBits   = 16;
//     u_int16_t   bits,      dataTmp;
//     int         i;

//     dataTmp = *dataPtr++;
//     for(i=0;i<sum;i++)
//     {
//         lengthTmp = lengthPtr[i] & 0x0f;
//         length = lengthTmp;
//         if(lengthTmp == 0)
//             buf[i] = 0;
//         else 
//         {
//             bits = 0;
//             if(fullBits < lengthTmp)
//             {
//                 bits = dataTmp >> (16 - fullBits);
//                 lengthTmp -= fullBits;
//                 bits <<= lengthTmp;
//                 fullBits = 16;
//                 dataTmp = *dataPtr++;
//             }
//             bits |= dataTmp >> (16 - lengthTmp);
//             fullBits -= lengthTmp;
//             dataTmp <<= lengthTmp;

//             if((bits >> (length - 1)))
//                 buf[i] = bits;
//             else 
//                 buf[i] = -(bits ^ (0xffff >> (16 - length)));
//         }
//     }

//     rle->value  = buf;
//     rle->valLen = rle->lenLen;
// }

// #ifdef  ZIGZAG
// static void de_zigzag(short *ori, short *dst)
// {
//     for(int i=0;i<64;i++)
//         dst[i] =   ori[zz_sequence[i]];
// }
// #endif  // #ifdef  ZIGZAG

// static void rleDecode(rleResult *rle)
// {
//     short       *dctTmp     = (short*)g_malloc0(sizeof(short)*rle->lenLen<<6);
//     short       *dctPtr     = dctTmp;
//     u_int8_t    *lengthPtr  = rle->length;
//     short       *dataPtr    = rle->value;
//     u_int32_t   lenLen      = rle->lenLen;
//     u_int32_t   blockLeft   = 64;
//     u_int8_t    lengthTmp;
//     int         i;
//     short       lastDC      = 0;
//     #ifdef  ZIGZAG
//     short       *zigzagTmp  = (short*)malloc(sizeof(short)*rle->lenLen<<6);
//     short       *zigzagPtr  = zigzagTmp;
//     #endif

//     for(i=0;i<lenLen;i++)
//     {
//         if(lengthTmp = (lengthPtr[i] >> 4))
//         {
//             dctPtr += lengthTmp;
//             *dctPtr++ = dataPtr[i];
//             blockLeft -= (lengthTmp + 1);
//         }
//         else 
//         {
//             if(dataPtr[i])
//             {
//                 *dctPtr++ = dataPtr[i];
//                 blockLeft --;
//             }
//             else 
//             {
//                 dctPtr += blockLeft;
//                 blockLeft = 0;
//             }
//         }
//         if(blockLeft == 0)
//         {
//             blockLeft = 64;
//             *(dctPtr - 64) += lastDC;
//             lastDC = *(dctPtr - 64);
//             #ifdef  ZIGZAG
//             de_zigzag(dctPtr-64,zigzagPtr);
//             zigzagPtr += 64;
//             #endif
//         }
//     }

//     #ifdef  ZIGZAG
//     free(dctTmp);
//     dctTmp  =   zigzagTmp;
//     #endif
//     free(rle->value);
//     rle->value = dctTmp;
//     rle->valLen = sizeof(short)*rle->lenLen<<6;
// }

// static short *decode_myjpeg(de_readPtr readPtr)
// // static short *decode_myjpeg(uint8_t *data)
// {
//     uint8_t     *ptr    = readPtr->in_d;
//     // uint8_t     *ptr    = data;
//     uint32_t    valSize;
//     memcpy(&valSize, ptr, sizeof(uint32_t));
//     ptr +=  sizeof(uint32_t);
//     uint8_t     *val    = ptr;
//     ptr +=  valSize;
//     uint8_t     ifCmpd  = *ptr++;
//     uint32_t    lenSize;
//     memcpy(&lenSize, ptr, sizeof(lenSize));
//     ptr +=  sizeof(uint32_t);
//     uint8_t     *len    = ptr;

//     uint8_t     *length = (uint8_t*)malloc(lenSize<<10);
//     rleResult   rle;
//     if(ifCmpd)
//     {
//         rle.lenLen = FSE_decompress(length, lenSize<<10, len, lenSize);
//         if(rle.lenLen <= 0)
//             printf("fail to decompress in *decode_myjpeg*.\n");
//         rle.length = length;
//     }
//     else 
//     {
//         rle.lenLen = lenSize;
//         rle.length = len;
//     }
//     rle.value = (short*)val;

//     vliDecode(&rle);
//     rleDecode(&rle);

//     free(length);

//     // readPtr->sizes[12] = rle.valLen;
//     return rle.value;
// }

// #ifdef COMPRESS_DELTA_INS
// static void decompress_delta_ins(de_readPtr readPtr)
// {
//     uint64_t    size    =   readPtr->sizes[0]*readPtr->sizes[1]+readPtr->sizes[2]*readPtr->sizes[3]*2;
//     if(readPtr->flag & 0x08)
//     {
//         uint8_t *cpx    =   (uint8_t*)malloc(size*sizeof(COPY_X));
//         readPtr->sizes[8]   =   FSE_decompress(cpx, size*sizeof(COPY_X), readPtr->x, readPtr->sizes[8]);
//         readPtr->x  =   cpx;
//     }
//     if(readPtr->flag & 0x04)
//     {
//         uint8_t *cpy    =   (uint8_t*)malloc(size*sizeof(COPY_Y));
//         readPtr->sizes[9]   =   FSE_decompress(cpy, size*sizeof(COPY_Y), readPtr->y, readPtr->sizes[9]);
//         readPtr->y  =   cpy;
//     }
//     if(readPtr->flag & 0x02)
//     {
//         uint8_t *cpl    =   (uint8_t*)malloc(size*sizeof(COPY_L));
//         readPtr->sizes[10]  =   FSE_decompress(cpl, size*sizeof(COPY_L), readPtr->cp_l, readPtr->sizes[10]);
//         readPtr->cp_l   =   cpl;
//     }
//     if(readPtr->flag & 0x01)
//     {
//         uint8_t *inl    =   (uint8_t*)malloc(size*sizeof(INSERT_L));
//         readPtr->sizes[11]  =   FSE_decompress(inl, size*sizeof(INSERT_L), readPtr->in_l, readPtr->sizes[11]);
//         readPtr->in_l   =   inl;
//     }
// }
// #endif

// // void* de_decode_thread(void *parameter)
// // {
// //     void        **arg    =   (void**)parameter;
// //     List        readList =   (List)arg[0];
// //     List        decdList =   (List)arg[1];
// //     de_readPtr  readPtr, readTmp;

// //     while(1)
// //     {
// //         pthread_mutex_lock(&readList->mutex);
// //         if(readList->counter == 0)
// //         {
// //             if(readList->ending)  break;
// //             else 
// //             {
// //                 pthread_cond_wait(&readList->rCond, &readList->mutex);
// //                 if(readList->counter == 0)    break;
// //             }
// //         }
// //         readPtr   =   readList->head;
// //         readList->head    =   NULL;
// //         readList->tail    =   NULL;
// //         readList->counter =   0;
// //         pthread_mutex_unlock(&readList->mutex);
// //         pthread_cond_signal(&readList->wCond);

// //         while(readPtr)
// //         {
// //             if(readPtr->sizes[12])
// //                 readPtr->in_d =   (uint8_t*)decode_myjpeg(readPtr->in_d);
// //             else 
// //                 readPtr->in_d =   NULL;
// //             #ifdef COMPRESS_DELTA_INS
// //             decompress_delta_ins(readPtr);
// //             #endif

// //             readTmp =   readPtr->next;
// //             readPtr->next   =   NULL;
// //             pthread_mutex_lock(&decdList->mutex);
// //             if(decdList->counter)
// //                 ((de_readPtr)decdList->tail)->next  =   readPtr;
// //             else 
// //                 decdList->head  =   readPtr;
// //             decdList->tail  =   readPtr;
// //             decdList->counter   ++;
// //             pthread_cond_signal(&decdList->rCond);
// //             // if(decdList->counter == OTHER_LIST_LEN)
// //             //     pthread_cond_wait(&decdList->wCond, &decdList->mutex);
// //             pthread_mutex_unlock(&decdList->mutex);

// //             readPtr =   readTmp;
// //         }
// //     }

// //     pthread_mutex_unlock(&readList->mutex);

// //     pthread_mutex_lock(&decdList->mutex);
// //     decdList->ending = 1;
// //     pthread_mutex_unlock(&decdList->mutex);
// //     pthread_cond_signal(&decdList->rCond);
// // }


// void* de_decode_thread(void *parameter)
// {
//     void        **arg    =   (void**)parameter;
//     List        readList =   (List)arg[0];
//     List        decdList =   (List)arg[1];
//     de_readPtr  readPtr;
//     pairPtr     pair;
//     #ifdef  PART_TIME
//     GTimer  *timer  =   g_timer_new();
//     #endif

//     while(1)
//     {
//         pthread_mutex_lock(&readList->mutex);
//         while(readList->counter == 0)
//         {
//             if(readList->ending == 2)  goto ESCAPE_LOOP;
//             pthread_cond_wait(&readList->rCond, &readList->mutex);
//         }
//         pair   =   readList->head;
//         readList->head    =   pair->next;
//         readList->size    +=    pair->mem_size;
//         readList->counter --;
//         pthread_cond_signal(&readList->wCond);
//         pthread_mutex_unlock(&readList->mutex);

//         #ifdef  PART_TIME
//         g_timer_start(timer);
//         #endif

//         if(pair)
//         {
//             readPtr =   pair->target;
//             if(readPtr->sizes[12])
//             {
//                 pair->mem_size -= readPtr->mem_size;
//                 readPtr->mem_size -= readPtr->sizes[12];
//                 readPtr->in_d =   (uint8_t*)decode_myjpeg(readPtr);
//                 readPtr->mem_size += readPtr->sizes[12];
//                 pair->mem_size += readPtr->mem_size;
//             }
//             else 
//                 readPtr->in_d =   NULL;
//             #ifdef COMPRESS_DELTA_INS
//             pair->mem_size -= readPtr->mem_size;
//             readPtr->mem_size -= (readPtr->sizes[8] + readPtr->sizes[9] + readPtr->sizes[10] + readPtr->sizes[11]);
//             decompress_delta_ins(readPtr);
//             readPtr->mem_size += (readPtr->sizes[8] + readPtr->sizes[9] + readPtr->sizes[10] + readPtr->sizes[11]);
//             pair->mem_size += readPtr->mem_size;
//             #endif

//             #ifdef PART_TIME
//             pthread_mutex_lock(&rejpeg_time_mutex);
//             rejpeg_time   +=  g_timer_elapsed(timer, NULL);
//             pthread_mutex_unlock(&rejpeg_time_mutex);
//             #endif

//             pair->next   =   NULL;
//             pthread_mutex_lock(&decdList->mutex);
//             while(decdList->size < pair->mem_size)
//                 pthread_cond_wait(&decdList->wCond, &decdList->mutex);
//             if(decdList->counter)
//                 ((pairPtr)decdList->tail)->next =   pair;
//             else 
//                 decdList->head  =   pair;
//             decdList->tail  =   pair;
//             decdList->counter   ++;
//             decdList->size  -=  pair->mem_size;
//             pthread_cond_signal(&decdList->rCond);
//             pthread_mutex_unlock(&decdList->mutex);
//         }
//     }

//     ESCAPE_LOOP:
//     pthread_mutex_unlock(&readList->mutex);

//     #ifdef PART_TIME
//     g_timer_destroy(timer);
//     #endif

//     pthread_mutex_lock(&decdList->mutex);
//     decdList->ending ++;
//     pthread_cond_signal(&decdList->rCond);
//     pthread_mutex_unlock(&decdList->mutex);
// }





























#include "rejpeg.h"

#ifdef PART_TIME
extern double rejpeg_time;
extern pthread_mutex_t rejpeg_time_mutex;
#endif

#ifndef JPEG_SEPA_COMP
typedef struct 
{
    short       *value;
    u_int8_t    *length;
    u_int32_t   valLen, lenLen;

}   rleResult;
#else
typedef struct 
{
    short       *value, *dc_value;
    u_int8_t    *length, *dc_length;
    u_int32_t   valLen, lenLen, dc_len, dc_valLen;

}   rleResult;
#endif

#ifdef ZIGZAG
char zz_sequence[64] = 
{
    0 ,1 ,5 ,6 ,14,15,27,28,
    2 ,4 ,7 ,13,16,26,29,42,
    3 ,8 ,12,17,25,30,41,43,
    9 ,11,18,24,31,40,44,53,
    10,19,23,32,39,45,52,54,
    20,22,33,38,46,51,55,60,
    21,34,37,47,50,56,59,61,
    35,36,48,49,57,58,62,63,
};

static short* zigzag(short *ptr)
{
    short   *tmp =  (short*)malloc(sizeof(short)<<6);
    int     i;

    for(i=0;i<64;i++)
    {
        tmp[zz_sequence[i]] =   ptr[i];
    }

    return tmp;
}
#endif  // #ifdef ZIGZAG

static rleResult rleEncode(GArray *dataPtr
    #ifdef JPEG_SEPA_COMP
    , uint32_t from, uint32_t componentLen
    #endif
)
{
    u_int32_t   valLen  = 0, lenLen = 0;
    #ifdef JPEG_SEPA_COMP
    u_int8_t    *length = (u_int8_t*)malloc(componentLen << 6), 
                *dc_length = (u_int8_t*)malloc(componentLen << 6);
    short       *value  = (short*)malloc(sizeof(short)*componentLen << 6),
                *dc_value = (short*)malloc(sizeof(short)*componentLen << 6);
    uint32_t    dc_len = 0;
    #else
    u_int8_t    *length = (u_int8_t*)malloc(dataPtr->len << 6);
    short       *value  = (short*)malloc(sizeof(short)*dataPtr->len << 6);
    #endif
    u_int32_t   zeroCounter  = 0;
    int         i, j, h;
    short       *ptr;
    short       lastDC  = 0, thisDC;
    rleResult   result;

    #ifndef JPEG_SEPA_COMP
    for(i=0;i<dataPtr->len;i++)
    #else
    for(i=0;i<componentLen;i++)
    #endif
    {
        #ifndef JPEG_SEPA_COMP
        // ptr = (short*)g_ptr_array_index(dataPtr,i);
        ptr = (short*)g_array_index(dataPtr, uint8_t*, i);
        #else
        // ptr = (short*)g_ptr_array_index(dataPtr,i+from);
        ptr = (short*)g_array_index(dataPtr,uint8_t*,i+from);
        #endif
        #ifdef  ZIGZAG
        ptr = zigzag(ptr);
        #endif
        thisDC = ptr[0];
        ptr[0] = thisDC - lastDC;
        lastDC = thisDC;

        for(h=63;h>=0;h--)
            if(ptr[h]) break;

        dc_value[dc_len] = ptr[0];
        dc_length[dc_len] = 0;
        dc_len++;

        for(j=1;j<=h;j++)
        {
            if(ptr[j])
            {
                value[valLen++] = ptr[j];
                length[lenLen++] = zeroCounter;
                zeroCounter = 0;
            }
            else 
            {
                if(zeroCounter == 15)
                {
                    value[valLen++] = 0;
                    length[lenLen++] = 15;
                    zeroCounter = 0;
                }
                else 
                    zeroCounter ++;
            }
        }
        if(h<63)
        {
            value[valLen++] = 0;
            length[lenLen++] = 0;
        }
        
        #ifdef  ZIGZAG
        free(ptr);
        #else
        ptr[0]  =   lastDC;
        #endif
    }

    result.length = length;
    result.value  = value;
    result.lenLen = lenLen;
    result.valLen = valLen;
    result.dc_length = dc_length;
    result.dc_value = dc_value;
    result.dc_len = dc_len;

    return result;
}

static void vliEncode(rleResult *rle, uint8_t *buf)
{
    u_int8_t    *lenPtr = rle->length;
    short       *valPtr = rle->value;
    u_int16_t   *data   = (u_int16_t*)buf;
    short       number;
    u_int16_t   _number_;
    u_int32_t   size    = rle->valLen;
    u_int8_t    bitLength; 
    u_int16_t   bits;
    int         dataPos = 0, emptyBits = 16, i;

    for(i=0;i<size;i++)
    {
        number   = valPtr[i];
        _number_ = abs(number);
        if     (_number_==0   )   bitLength = 0;
        else if(_number_==1   )   bitLength = 1;
        else if(_number_<=3   )   bitLength = 2;
        else if(_number_<=7   )   bitLength = 3;
        else if(_number_<=15  )   bitLength = 4;
        else if(_number_<=31  )   bitLength = 5;
        else if(_number_<=63  )   bitLength = 6;
        else if(_number_<=127 )   bitLength = 7;
        else if(_number_<=255 )   bitLength = 8;
        else if(_number_<=511 )   bitLength = 9;
        else if(_number_<=1023)   bitLength = 10;
        else if(_number_<=2047)   bitLength = 11;
        else if(_number_<=4095)   bitLength = 12;

        lenPtr[i] = (lenPtr[i] << 4) | bitLength;

        if(number)
        {
            if(number > 0)  bits = number;
            else bits = (-number) ^ (0xffff >> (16 - bitLength));

            if(emptyBits >= bitLength)
            {
                emptyBits -= bitLength;
                data[dataPos] |= bits << emptyBits;
            }
            else 
            {
                bitLength -= emptyBits;
                data[dataPos] |= bits >> bitLength;
                emptyBits = 16 - bitLength;
                data[++dataPos] |= bits << emptyBits;
            }
        }
    }

    if(size)
        rle->valLen  =   dataPos + 1;
    lenPtr  =   rle->dc_length;
    valPtr  =   rle->dc_value;
    emptyBits    =   16;
    size    =   rle->dc_len;
    dataPos +=  3;

    for(i=0;i<size;i++)
    {
        number   = valPtr[i];
        _number_ = abs(number);
        if     (_number_==0   )   bitLength = 0;
        else if(_number_==1   )   bitLength = 1;
        else if(_number_<=3   )   bitLength = 2;
        else if(_number_<=7   )   bitLength = 3;
        else if(_number_<=15  )   bitLength = 4;
        else if(_number_<=31  )   bitLength = 5;
        else if(_number_<=63  )   bitLength = 6;
        else if(_number_<=127 )   bitLength = 7;
        else if(_number_<=255 )   bitLength = 8;
        else if(_number_<=511 )   bitLength = 9;
        else if(_number_<=1023)   bitLength = 10;
        else if(_number_<=2047)   bitLength = 11;
        else if(_number_<=4095)   bitLength = 12;

        lenPtr[i] = (lenPtr[i] << 4) | bitLength;

        if(number)
        {
            if(number > 0)  bits = number;
            else bits = (-number) ^ (0xffff >> (16 - bitLength));

            if(emptyBits >= bitLength)
            {
                emptyBits -= bitLength;
                data[dataPos] |= bits << emptyBits;
            }
            else 
            {
                bitLength -= emptyBits;
                data[dataPos] |= bits >> bitLength;
                emptyBits = 16 - bitLength;
                data[++dataPos] |= bits << emptyBits;
            }
        }
    }

    free(rle->value);
    free(rle->dc_value);
    rle->value = data;
    if(size)
    {
        rle->dc_valLen = dataPos + 1 - rle->valLen - 2;
        // memcpy(data+rle->valLen, &(rle->dc_valLen), sizeof(rle->dc_valLen));
        *((uint32_t*)(data+rle->valLen)) = rle->dc_valLen;
    }
}

#ifdef ORIGINAL_HUFF
HUF_FSE* Huff_tree_transform(JHUFF_TBL *hufTab)
{
    if(!hufTab) return NULL;
    HUF_FSE *newTab    =   (HUF_FSE*)g_malloc0(sizeof(HUF_FSE)*256);
    uint16_t    symbol  =   0;
    uint32_t    count   =   0;
    for(int i=1; i<=16; i++,symbol<<=1)
    {
        for(int j=0; j<hufTab->bits[i]; j++)
        {
            newTab[hufTab->huffval[count]].nbBits = i;
            newTab[hufTab->huffval[count]].val  =   symbol;
            symbol ++;
            count  ++;
        }
    }
    return newTab;
}
#endif

static int recompressed_by_myjpeg(GArray *dataPtr, uint8_t *buf, uint32_t bufSize
    #ifdef JPEG_SEPA_COMP
    , uint32_t from, uint32_t compoentLen
    #endif
    #ifdef ORIGINAL_HUFF
    , JHUFF_TBL *hufTab
    #endif
)
{
    if(compoentLen <= 0)    return 0;
    uint32_t size   =   sizeof(uint32_t);
    uint8_t *ptr    =   buf + size;
    rleResult rle   =   rleEncode(dataPtr
        #ifdef JPEG_SEPA_COMP
        ,from,compoentLen
        #endif
        );
    vliEncode(&rle,ptr);

    uint32_t valSize=   rle.valLen*sizeof(uint16_t);
    memcpy(buf, &valSize, sizeof(valSize));
    ptr += valSize + (rle.dc_valLen+2)*sizeof(uint16_t);
    size+= valSize + (rle.dc_valLen+2)*sizeof(uint16_t);

    size    +=  (sizeof(uint8_t) + sizeof(uint32_t))*2;
    uint8_t *pre = ptr;
    ptr += (sizeof(uint8_t) + sizeof(uint32_t))*2;
    if(bufSize <= size)
    {
        free(rle.length);
        free(rle.dc_length);
        return -1;
    }
    #ifdef ORIGINAL_HUFF
    HUF_FSE *huffTree = Huff_tree_transform(hufTab+1);
    uint32_t lenLen = HUF_compress_JPEGtab(rle.length, rle.lenLen, ptr, bufSize - size, huffTree);
    free(huffTree);
    #else
    uint32_t lenLen = entropy_compress(rle.length, rle.lenLen, ptr, bufSize - size);
    #endif
    if(lenLen > 0)
        *pre++ = 1;
    else 
    {
        if(rle.lenLen > (bufSize - size))
        {
            free(rle.length);
            free(rle.dc_length);
            return -1;
        }
        *pre++ = 0;
        memcpy(ptr, rle.length, rle.lenLen);
        lenLen = rle.lenLen;
    }
    ptr  += lenLen;
    size += lenLen;
    memcpy(pre, &lenLen, sizeof(lenLen));
    pre +=  sizeof(uint32_t);

    #ifdef ORIGINAL_HUFF
    huffTree = Huff_tree_transform(hufTab);
    lenLen = HUF_compress_JPEGtab(rle.dc_length,rle.dc_len,ptr,bufSize-size, huffTree);
    free(huffTree);
    #else
    lenLen = entropy_compress(rle.dc_length,rle.dc_len,ptr,bufSize-size);
    #endif
    if(lenLen > 0)
        *pre++ = 1;
    else 
    {
        if(rle.dc_len > (bufSize - size))
        {
            free(rle.length);
            free(rle.dc_length);
            return -1;
        }
        *pre++ = 0;
        memcpy(ptr, rle.dc_length, rle.dc_len);
        lenLen = rle.dc_len;
    }
    ptr  += lenLen;
    size += lenLen;
    memcpy(pre, &lenLen, sizeof(lenLen));

    free(rle.length);
    free(rle.dc_length);

    return size;
}

// static rleResult rleEncode(GArray *dataPtr
//     #ifdef JPEG_SEPA_COMP
//     , uint32_t from, uint32_t componentLen
//     #endif
// )
// {
//     u_int32_t   valLen  = 0, lenLen = 0;
//     #ifdef JPEG_SEPA_COMP
//     u_int8_t    *length = (u_int8_t*)malloc(componentLen << 6);
//     short       *value  = (short*)malloc(sizeof(short)*componentLen << 6);
//     #else
//     u_int8_t    *length = (u_int8_t*)malloc(dataPtr->len << 6);
//     short       *value  = (short*)malloc(sizeof(short)*dataPtr->len << 6);
//     #endif
//     u_int32_t   zeroCounter  = 0;
//     int         i, j, h;
//     short       *ptr;
//     short       lastDC  = 0, thisDC;
//     rleResult   result;

//     #ifndef JPEG_SEPA_COMP
//     for(i=0;i<dataPtr->len;i++)
//     #else
//     for(i=0;i<componentLen;i++)
//     #endif
//     {
//         #ifndef JPEG_SEPA_COMP
//         // ptr = (short*)g_ptr_array_index(dataPtr,i);
//         ptr = (short*)g_array_index(dataPtr, uint8_t*, i);
//         #else
//         // ptr = (short*)g_ptr_array_index(dataPtr,i+from);
//         ptr = (short*)g_array_index(dataPtr,uint8_t*,i+from);
//         #endif
//         #ifdef  ZIGZAG
//         ptr = zigzag(ptr);
//         #endif
//         thisDC = ptr[0];
//         ptr[0] = thisDC - lastDC;
//         lastDC = thisDC;

//         for(h=63;h>=0;h--)
//             if(ptr[h]) break;

//         for(j=0;j<=h;j++)
//         {
//             if(ptr[j])
//             {
//                 value[valLen++] = ptr[j];
//                 length[lenLen++] = zeroCounter;
//                 zeroCounter = 0;
//             }
//             else 
//             {
//                 if(zeroCounter == 15)
//                 {
//                     value[valLen++] = 0;
//                     length[lenLen++] = 15;
//                     zeroCounter = 0;
//                 }
//                 else 
//                     zeroCounter ++;
//             }
//         }
//         if(h<63)
//         {
//             value[valLen++] = 0;
//             length[lenLen++] = 0;
//         }
        
//         #ifdef  ZIGZAG
//         free(ptr);
//         #else
//         ptr[0]  =   lastDC;
//         #endif
//     }

//     result.length = length;
//     result.value  = value;
//     result.lenLen = lenLen;
//     result.valLen = valLen;

//     return result;
// }

// static void vliEncode(rleResult *rle, uint8_t *buf)
// {
//     u_int8_t    *lenPtr = rle->length;
//     short       *valPtr = rle->value;
//     u_int16_t   *data   = (u_int16_t*)buf;
//     short       number;
//     u_int16_t   _number_;
//     u_int32_t   size    = rle->valLen;
//     u_int8_t    bitLength; 
//     u_int16_t   bits;
//     int         dataPos = 0, emptyBits = 16, i;

//     for(i=0;i<size;i++)
//     {
//         number   = valPtr[i];
//         _number_ = abs(number);
//         if     (_number_==0   )   bitLength = 0;
//         else if(_number_==1   )   bitLength = 1;
//         else if(_number_<=3   )   bitLength = 2;
//         else if(_number_<=7   )   bitLength = 3;
//         else if(_number_<=15  )   bitLength = 4;
//         else if(_number_<=31  )   bitLength = 5;
//         else if(_number_<=63  )   bitLength = 6;
//         else if(_number_<=127 )   bitLength = 7;
//         else if(_number_<=255 )   bitLength = 8;
//         else if(_number_<=511 )   bitLength = 9;
//         else if(_number_<=1023)   bitLength = 10;
//         else if(_number_<=2047)   bitLength = 11;
//         else if(_number_<=4095)   bitLength = 12;

//         lenPtr[i] = (lenPtr[i] << 4) | bitLength;

//         if(number)
//         {
//             if(number > 0)  bits = number;
//             else bits = (-number) ^ (0xffff >> (16 - bitLength));

//             if(emptyBits >= bitLength)
//             {
//                 emptyBits -= bitLength;
//                 data[dataPos] |= bits << emptyBits;
//             }
//             else 
//             {
//                 bitLength -= emptyBits;
//                 data[dataPos] |= bits >> bitLength;
//                 emptyBits = 16 - bitLength;
//                 data[++dataPos] |= bits << emptyBits;
//             }
//         }
//     }

//     free(rle->value);
//     rle->value = data;
//     if(size)
//         rle->valLen = dataPos + 1;
// }

/**
 * [4 bytes] (the size of value data : n);
 * [n bytes] (value data);
 * [1 byte ] (if length data has been compressed);
 * [4 bytes] (the size of length data : m);
 * [m bytes] (length data).
 * */
// static int recompressed_by_myjpeg(GPtrArray *dataPtr, uint8_t *buf, uint32_t bufSize
// static int recompressed_by_myjpeg(GArray *dataPtr, uint8_t *buf, uint32_t bufSize
//     #ifdef JPEG_SEPA_COMP
//     , uint32_t from, uint32_t compoentLen
//     #endif
// )
// {
//     uint32_t size   =   sizeof(uint32_t);
//     uint8_t *ptr    =   buf + size;
//     rleResult rle   =   rleEncode(dataPtr
//         #ifdef JPEG_SEPA_COMP
//         ,from,compoentLen
//         #endif
//         );
//     vliEncode(&rle,ptr);

//     uint32_t valSize=   rle.valLen*sizeof(uint16_t);
//     memcpy(buf, &valSize, sizeof(valSize));
//     ptr += valSize;
//     size+= valSize;

//     size    +=  (sizeof(uint8_t) + sizeof(uint32_t));
//     uint8_t *pre = ptr;
//     ptr += (sizeof(uint8_t) + sizeof(uint32_t));
//     uint32_t lenLen = entropy_compress(rle.length, rle.lenLen, ptr, bufSize - size);
//     if(lenLen > 0)
//         *pre++ = 1;
//     else 
//     {
//         if(rle.lenLen > (bufSize - size))
//         {
//             free(rle.length);
//             return -1;
//         }
//         *pre++ = 0;
//         memcpy(ptr, rle.length, rle.lenLen);
//         lenLen = rle.lenLen;
//     }
//     ptr  += lenLen;
//     size += lenLen;
//     memcpy(pre, &lenLen, sizeof(lenLen));

//     free(rle.length);

//     return size;
// }

static rejpegResPtr rejpeg_a_single_img(dedupResPtr dedupPtr)
{
    uint32_t    bfSize  =   dedupPtr->insert_p->len << 7;
    uint8_t     *buf    =   NULL;
    int         size    =   0;
    rejpegResPtr    rejpegPtr   =   (rejpegResPtr)malloc(sizeof(rejpegResNode));

    for( ; bfSize>0; bfSize <<= 1)
    {
        #ifndef JPEG_SEPA_COMP
        buf     =   (uint8_t*)g_malloc0(bfSize);
        size    =   recompressed_by_myjpeg(dedupPtr->insert_p, buf, bfSize);
        if(size < 0)
            free(buf);
        else 
            break;
        #else
        size = 0;
        int sizeTmp;
        buf     =   (uint8_t*)g_malloc0(bfSize);
        sizeTmp =   recompressed_by_myjpeg(dedupPtr->insert_p, buf, bfSize, 0, dedupPtr->p_counter[0]
            #ifdef ORIGINAL_HUFF
            , dedupPtr->hufTab
            #endif
        );
        if(sizeTmp < 0)
        {
            free(buf);
            continue;
        }
        size += sizeTmp;
        sizeTmp =   recompressed_by_myjpeg(dedupPtr->insert_p, buf+size, bfSize-size, dedupPtr->p_counter[0], dedupPtr->p_counter[1]
            #ifdef ORIGINAL_HUFF
            , dedupPtr->hufTab + 2
            #endif
        );
        if(sizeTmp < 0)
        {
            free(buf);
            continue;
        }
        size += sizeTmp;
        sizeTmp =   recompressed_by_myjpeg(dedupPtr->insert_p, buf+size, bfSize-size, dedupPtr->p_counter[0]+dedupPtr->p_counter[1], dedupPtr->p_counter[2]
            #ifdef ORIGINAL_HUFF
            , dedupPtr->hufTab + 4
            #endif
        );
        if(sizeTmp < 0)
        {
            free(buf);
            continue;
        }
        size += sizeTmp;
        break;
        #endif
    }

    rejpegPtr->dedupRes     =   dedupPtr;
    rejpegPtr->rejpegRes    =   buf;
    rejpegPtr->rejpegSize   =   size;
    rejpegPtr->mem_size     =   size + sizeof(rejpegResNode) + dedupPtr->mem_size;

    return  rejpegPtr;
}

#ifdef COMPRESS_DELTA_INS
static void compress_delta_ins(rejpegResPtr rejpeg)
{
    uint32_t    cpxSize =   rejpeg->dedupRes->copy_x->len*sizeof(COPY_X),
                cpySize =   rejpeg->dedupRes->copy_y->len*sizeof(COPY_Y),
                cplSize =   rejpeg->dedupRes->copy_l->len*sizeof(COPY_L),
                inlSize =   rejpeg->dedupRes->insert_l->len*sizeof(INSERT_L);
    rejpeg->cpx =   (uint8_t*)malloc(cpxSize);
    rejpeg->cpy =   (uint8_t*)malloc(cpySize);
    rejpeg->cpl =   (uint8_t*)malloc(cplSize);
    rejpeg->inl =   (uint8_t*)malloc(inlSize);

    rejpeg->cpxSize =   entropy_compress(rejpeg->dedupRes->copy_x->data, cpxSize, rejpeg->cpx, cpxSize);
    if(rejpeg->cpxSize > 0)
        rejpeg->flag    =   0x08;
    else 
    {
        rejpeg->flag    =   0;
        free(rejpeg->cpx);
        rejpeg->cpx     =   NULL;
        rejpeg->cpxSize =   cpxSize;
    }
    rejpeg->cpySize =   entropy_compress(rejpeg->dedupRes->copy_y->data, cpySize, rejpeg->cpy, cpySize);
    if(rejpeg->cpySize > 0)
        rejpeg->flag    |=  0x04;
    else 
    {
        free(rejpeg->cpy);
        rejpeg->cpy     =   NULL;
        rejpeg->cpySize =   cpySize;
    }
    rejpeg->cplSize =   entropy_compress(rejpeg->dedupRes->copy_l->data, cplSize, rejpeg->cpl, cplSize);
    if(rejpeg->cplSize > 0)
        rejpeg->flag    |=  0x02;
    else 
    {
        free(rejpeg->cpl);
        rejpeg->cpl     =   NULL;
        rejpeg->cplSize =   cplSize;
    }
    rejpeg->inlSize =   entropy_compress(rejpeg->dedupRes->insert_l->data, inlSize, rejpeg->inl, inlSize);
    if(rejpeg->inlSize > 0)
        rejpeg->flag    |=  0x01;
    else 
    {
        free(rejpeg->inl);
        rejpeg->inl     =   NULL;
        rejpeg->inlSize =   inlSize;
    }
}
#endif

void* rejpeg_thread(void *parameter)
{
    void        **arg       =   (void**)parameter;
    List        dedupList   =   (List)arg[0];
    List        rejpegList  =   (List)arg[1];
    char        *outPath    =   (char*)arg[2];
    dedupResPtr dedupPtr;
    rejpegResPtr    rejpegPtr;
    #ifdef PART_TIME
    GTimer      *timer      =   g_timer_new();
    #endif

    while(1)
    {
        pthread_mutex_lock(&dedupList->mutex);
        while(dedupList->counter == 0)
        {
            if(dedupList->ending == MIDDLE_THREAD_NUM) goto ESCAPE_LOOP;
            pthread_cond_wait(&dedupList->rCond, &dedupList->mutex);
        }
        dedupPtr = dedupList->head;
        dedupList->head =   dedupPtr->next;
        dedupList->counter --;
        dedupList->size +=  dedupPtr->mem_size;
        for(int i=0; i<MIDDLE_THREAD_NUM; i++)
        pthread_cond_signal(&dedupList->wCond);
        pthread_mutex_unlock(&dedupList->mutex);

        if(dedupPtr)
        {
            #ifdef PART_TIME
            g_timer_start(timer);
            #endif

            rejpegPtr   =   rejpeg_a_single_img(dedupPtr);
            pthread_mutex_lock(&dedupPtr->node->mutex);
            dedupPtr->node->link --;
            pthread_mutex_unlock(&dedupPtr->node->mutex);
            #ifdef COMPRESS_DELTA_INS
            compress_delta_ins(rejpegPtr);
            rejpegPtr->mem_size +=  (rejpegPtr->cpxSize + rejpegPtr->cpySize + rejpegPtr->cplSize + rejpegPtr->inlSize);
            #endif

            #ifdef PART_TIME
            pthread_mutex_lock(&rejpeg_time_mutex);
            rejpeg_time +=  g_timer_elapsed(timer, NULL);
            pthread_mutex_unlock(&rejpeg_time_mutex);
            #endif

            rejpegPtr->next =   NULL;
            pthread_mutex_lock(&rejpegList->mutex);
            while(rejpegList->size < rejpegPtr->mem_size)
                pthread_cond_wait(&rejpegList->wCond, &rejpegList->mutex);
            if(rejpegList->counter)
                ((rejpegResPtr)rejpegList->tail)->next  =   rejpegPtr;
            else 
                rejpegList->head    =   rejpegPtr;
            rejpegList->tail    =   rejpegPtr;
            rejpegList->counter ++;
            rejpegList->size    -=  rejpegPtr->mem_size;
            pthread_cond_signal(&rejpegList->rCond);
            pthread_mutex_unlock(&rejpegList->mutex);
        }
    }

    ESCAPE_LOOP:
    pthread_mutex_unlock(&dedupList->mutex);

    #ifdef PART_TIME
    g_timer_destroy(timer);
    #endif

    pthread_mutex_lock(&rejpegList->mutex);
    rejpegList->ending  ++;
    for(int i=0; i<WRITE_THREAD_NUM; i++)
        pthread_cond_signal(&rejpegList->rCond);
    pthread_mutex_unlock(&rejpegList->mutex);
}

void* tra_rejpeg_thread(void *parameter)
{
    void        **arg       =   (void**)parameter;
    List        dedupList   =   (List)arg[0];
    List        rejpegList  =   (List)arg[1];
    char        *outPath    =   (char*)arg[2];
    dedupResPtr dedupPtr;
    rejpegResPtr    rejpegPtr;
    #ifdef PART_TIME
    GTimer      *timer      =   g_timer_new();
    #endif

    while(1)
    {
        pthread_mutex_lock(&dedupList->mutex);
        while(dedupList->counter == 0)
        {
            if(dedupList->ending) goto ESCAPE_LOOP;
            pthread_cond_wait(&dedupList->rCond, &dedupList->mutex);
        }
        dedupPtr = dedupList->head;
        dedupList->head =   dedupPtr->next;
        dedupList->counter --;
        dedupList->size +=  dedupPtr->mem_size;
        pthread_cond_signal(&dedupList->wCond);
        pthread_mutex_unlock(&dedupList->mutex);

        if(dedupPtr)
        {
            #ifdef PART_TIME
            g_timer_start(timer);
            #endif

            rejpegPtr   =   (rejpegResPtr)malloc(sizeof(rejpegResNode));
            rejpegPtr->dedupRes     =   dedupPtr;
            rejpegPtr->rejpegRes    =   NULL;
            rejpegPtr->rejpegSize   =   0;
            rejpegPtr->mem_size     =   sizeof(rejpegResNode) + dedupPtr->mem_size;
            uint8_t     *fseBuffer = (uint8_t*)malloc(dedupPtr->v_counter);
            dedupPtr->y_counter =   entropy_compress(
                dedupPtr->insert_l,dedupPtr->v_counter,fseBuffer,dedupPtr->v_counter);
            if(dedupPtr->y_counter > 0)
            {
                free(dedupPtr->insert_l);
                dedupPtr->v_counter = dedupPtr->y_counter;
                dedupPtr->insert_l = (GArray*)fseBuffer;
            }
            else    free(fseBuffer);

            pthread_mutex_lock(&dedupPtr->node->mutex);
            dedupPtr->node->link --;
            pthread_mutex_unlock(&dedupPtr->node->mutex);

            #ifdef PART_TIME
            pthread_mutex_lock(&rejpeg_time_mutex);
            rejpeg_time +=  g_timer_elapsed(timer, NULL);
            pthread_mutex_unlock(&rejpeg_time_mutex);
            #endif

            rejpegPtr->next =   NULL;
            pthread_mutex_lock(&rejpegList->mutex);
            while(rejpegList->size < rejpegPtr->mem_size)
                pthread_cond_wait(&rejpegList->wCond, &rejpegList->mutex);
            if(rejpegList->counter)
                ((rejpegResPtr)rejpegList->tail)->next  =   rejpegPtr;
            else 
                rejpegList->head    =   rejpegPtr;
            rejpegList->tail    =   rejpegPtr;
            rejpegList->counter ++;
            rejpegList->size    -=  rejpegPtr->mem_size;
            pthread_cond_signal(&rejpegList->rCond);
            pthread_mutex_unlock(&rejpegList->mutex);
        }
    }

    ESCAPE_LOOP:
    pthread_mutex_unlock(&dedupList->mutex);

    #ifdef PART_TIME
    g_timer_destroy(timer);
    #endif

    pthread_mutex_lock(&rejpegList->mutex);
    rejpegList->ending  ++;
    for(int i=0; i<WRITE_THREAD_NUM; i++)
        pthread_cond_signal(&rejpegList->rCond);
    pthread_mutex_unlock(&rejpegList->mutex);
}

/*-------------------------------------------------------------------------*/

static void vliDecode(rleResult *rle)
{
    u_int32_t   sum        = rle->lenLen;
    u_int8_t    lengthTmp, length;
    u_int8_t    *lengthPtr = rle->length;
    u_int16_t   *dataPtr   = rle->value;
    short       *buf       = (short*)malloc(sizeof(short)*sum);
    int         fullBits   = 16;
    u_int16_t   bits,      dataTmp;
    int         i;

    dataTmp = *dataPtr++;
    for(i=0;i<sum;i++)
    {
        lengthTmp = lengthPtr[i] & 0x0f;
        length = lengthTmp;
        if(lengthTmp == 0)
            buf[i] = 0;
        else 
        {
            bits = 0;
            if(fullBits < lengthTmp)
            {
                bits = dataTmp >> (16 - fullBits);
                lengthTmp -= fullBits;
                bits <<= lengthTmp;
                fullBits = 16;
                dataTmp = *dataPtr++;
            }
            bits |= dataTmp >> (16 - lengthTmp);
            fullBits -= lengthTmp;
            dataTmp <<= lengthTmp;

            if((bits >> (length - 1)))
                buf[i] = bits;
            else 
                buf[i] = -(bits ^ (0xffff >> (16 - length)));
        }
    }

    rle->value  = buf;
    rle->valLen = rle->lenLen;
}

#ifdef  ZIGZAG
static void de_zigzag(short *ori, short *dst)
{
    for(int i=0;i<64;i++)
        dst[i] =   ori[zz_sequence[i]];
}
#endif  // #ifdef  ZIGZAG

static void rleDecode(rleResult *rle)
{
    short       *dctTmp     = (short*)g_malloc0(sizeof(short)*rle->lenLen<<6);
    short       *dctPtr     = dctTmp;
    u_int8_t    *lengthPtr  = rle->length;
    short       *dataPtr    = rle->value;
    u_int32_t   lenLen      = rle->lenLen;
    u_int32_t   blockLeft   = 64;
    u_int8_t    lengthTmp;
    int         i;
    short       lastDC      = 0;
    #ifdef  ZIGZAG
    short       *zigzagTmp  = (short*)malloc(sizeof(short)*rle->lenLen<<6);
    short       *zigzagPtr  = zigzagTmp;
    #endif

    for(i=0;i<lenLen;i++)
    {
        if(lengthTmp = (lengthPtr[i] >> 4))
        {
            dctPtr += lengthTmp;
            *dctPtr++ = dataPtr[i];
            blockLeft -= (lengthTmp + 1);
        }
        else 
        {
            if(dataPtr[i])
            {
                *dctPtr++ = dataPtr[i];
                blockLeft --;
            }
            else 
            {
                dctPtr += blockLeft;
                blockLeft = 0;
            }
        }
        if(blockLeft == 0)
        {
            blockLeft = 64;
            *(dctPtr - 64) += lastDC;
            lastDC = *(dctPtr - 64);
            #ifdef  ZIGZAG
            de_zigzag(dctPtr-64,zigzagPtr);
            zigzagPtr += 64;
            #endif
        }
    }

    #ifdef  ZIGZAG
    free(dctTmp);
    dctTmp  =   zigzagTmp;
    #endif
    free(rle->value);
    rle->value = dctTmp;
    rle->valLen = sizeof(short)*rle->lenLen<<6;
}

static short *decode_myjpeg(de_readPtr readPtr)
// static short *decode_myjpeg(uint8_t *data)
{
    uint8_t     *ptr    = readPtr->in_d;
    // uint8_t     *ptr    = data;
    uint32_t    valSize;
    memcpy(&valSize, ptr, sizeof(uint32_t));
    ptr +=  sizeof(uint32_t);
    uint8_t     *val    = ptr;
    ptr +=  valSize;
    uint8_t     ifCmpd  = *ptr++;
    uint32_t    lenSize;
    memcpy(&lenSize, ptr, sizeof(lenSize));
    ptr +=  sizeof(uint32_t);
    uint8_t     *len    = ptr;

    uint8_t     *length = (uint8_t*)malloc(lenSize<<10);
    rleResult   rle;
    if(ifCmpd)
    {
        rle.lenLen = FSE_decompress(length, lenSize<<10, len, lenSize);
        if(rle.lenLen <= 0)
            printf("fail to decompress in *decode_myjpeg*.\n");
        rle.length = length;
    }
    else 
    {
        rle.lenLen = lenSize;
        rle.length = len;
    }
    rle.value = (short*)val;

    vliDecode(&rle);
    rleDecode(&rle);

    free(length);

    // readPtr->sizes[12] = rle.valLen;
    return rle.value;
}

#ifdef COMPRESS_DELTA_INS
static void decompress_delta_ins(de_readPtr readPtr)
{
    uint64_t    size    =   readPtr->sizes[0]*readPtr->sizes[1]+readPtr->sizes[2]*readPtr->sizes[3]*2;
    if(readPtr->flag & 0x08)
    {
        uint8_t *cpx    =   (uint8_t*)malloc(size*sizeof(COPY_X));
        readPtr->sizes[8]   =   FSE_decompress(cpx, size*sizeof(COPY_X), readPtr->x, readPtr->sizes[8]);
        readPtr->x  =   cpx;
    }
    if(readPtr->flag & 0x04)
    {
        uint8_t *cpy    =   (uint8_t*)malloc(size*sizeof(COPY_Y));
        readPtr->sizes[9]   =   FSE_decompress(cpy, size*sizeof(COPY_Y), readPtr->y, readPtr->sizes[9]);
        readPtr->y  =   cpy;
    }
    if(readPtr->flag & 0x02)
    {
        uint8_t *cpl    =   (uint8_t*)malloc(size*sizeof(COPY_L));
        readPtr->sizes[10]  =   FSE_decompress(cpl, size*sizeof(COPY_L), readPtr->cp_l, readPtr->sizes[10]);
        readPtr->cp_l   =   cpl;
    }
    if(readPtr->flag & 0x01)
    {
        uint8_t *inl    =   (uint8_t*)malloc(size*sizeof(INSERT_L));
        readPtr->sizes[11]  =   FSE_decompress(inl, size*sizeof(INSERT_L), readPtr->in_l, readPtr->sizes[11]);
        readPtr->in_l   =   inl;
    }
}
#endif

// void* de_decode_thread(void *parameter)
// {
//     void        **arg    =   (void**)parameter;
//     List        readList =   (List)arg[0];
//     List        decdList =   (List)arg[1];
//     de_readPtr  readPtr, readTmp;

//     while(1)
//     {
//         pthread_mutex_lock(&readList->mutex);
//         if(readList->counter == 0)
//         {
//             if(readList->ending)  break;
//             else 
//             {
//                 pthread_cond_wait(&readList->rCond, &readList->mutex);
//                 if(readList->counter == 0)    break;
//             }
//         }
//         readPtr   =   readList->head;
//         readList->head    =   NULL;
//         readList->tail    =   NULL;
//         readList->counter =   0;
//         pthread_mutex_unlock(&readList->mutex);
//         pthread_cond_signal(&readList->wCond);

//         while(readPtr)
//         {
//             if(readPtr->sizes[12])
//                 readPtr->in_d =   (uint8_t*)decode_myjpeg(readPtr->in_d);
//             else 
//                 readPtr->in_d =   NULL;
//             #ifdef COMPRESS_DELTA_INS
//             decompress_delta_ins(readPtr);
//             #endif

//             readTmp =   readPtr->next;
//             readPtr->next   =   NULL;
//             pthread_mutex_lock(&decdList->mutex);
//             if(decdList->counter)
//                 ((de_readPtr)decdList->tail)->next  =   readPtr;
//             else 
//                 decdList->head  =   readPtr;
//             decdList->tail  =   readPtr;
//             decdList->counter   ++;
//             pthread_cond_signal(&decdList->rCond);
//             // if(decdList->counter == OTHER_LIST_LEN)
//             //     pthread_cond_wait(&decdList->wCond, &decdList->mutex);
//             pthread_mutex_unlock(&decdList->mutex);

//             readPtr =   readTmp;
//         }
//     }

//     pthread_mutex_unlock(&readList->mutex);

//     pthread_mutex_lock(&decdList->mutex);
//     decdList->ending = 1;
//     pthread_mutex_unlock(&decdList->mutex);
//     pthread_cond_signal(&decdList->rCond);
// }


void* de_decode_thread(void *parameter)
{
    void        **arg    =   (void**)parameter;
    List        readList =   (List)arg[0];
    List        decdList =   (List)arg[1];
    de_readPtr  readPtr;
    pairPtr     pair;
    #ifdef  PART_TIME
    GTimer  *timer  =   g_timer_new();
    #endif

    while(1)
    {
        pthread_mutex_lock(&readList->mutex);
        while(readList->counter == 0)
        {
            if(readList->ending == 2)  goto ESCAPE_LOOP;
            pthread_cond_wait(&readList->rCond, &readList->mutex);
        }
        pair   =   readList->head;
        readList->head    =   pair->next;
        readList->size    +=    pair->mem_size;
        readList->counter --;
        pthread_cond_signal(&readList->wCond);
        pthread_mutex_unlock(&readList->mutex);

        #ifdef  PART_TIME
        g_timer_start(timer);
        #endif

        if(pair)
        {
            readPtr =   pair->target;
            if(readPtr->sizes[12])
            {
                pair->mem_size -= readPtr->mem_size;
                readPtr->mem_size -= readPtr->sizes[12];
                readPtr->in_d =   (uint8_t*)decode_myjpeg(readPtr);
                readPtr->mem_size += readPtr->sizes[12];
                pair->mem_size += readPtr->mem_size;
            }
            else 
                readPtr->in_d =   NULL;
            #ifdef COMPRESS_DELTA_INS
            pair->mem_size -= readPtr->mem_size;
            readPtr->mem_size -= (readPtr->sizes[8] + readPtr->sizes[9] + readPtr->sizes[10] + readPtr->sizes[11]);
            decompress_delta_ins(readPtr);
            readPtr->mem_size += (readPtr->sizes[8] + readPtr->sizes[9] + readPtr->sizes[10] + readPtr->sizes[11]);
            pair->mem_size += readPtr->mem_size;
            #endif

            #ifdef PART_TIME
            pthread_mutex_lock(&rejpeg_time_mutex);
            rejpeg_time   +=  g_timer_elapsed(timer, NULL);
            pthread_mutex_unlock(&rejpeg_time_mutex);
            #endif

            pair->next   =   NULL;
            pthread_mutex_lock(&decdList->mutex);
            while(decdList->size < pair->mem_size)
                pthread_cond_wait(&decdList->wCond, &decdList->mutex);
            if(decdList->counter)
                ((pairPtr)decdList->tail)->next =   pair;
            else 
                decdList->head  =   pair;
            decdList->tail  =   pair;
            decdList->counter   ++;
            decdList->size  -=  pair->mem_size;
            pthread_cond_signal(&decdList->rCond);
            pthread_mutex_unlock(&decdList->mutex);
        }
    }

    ESCAPE_LOOP:
    pthread_mutex_unlock(&readList->mutex);

    #ifdef PART_TIME
    g_timer_destroy(timer);
    #endif

    pthread_mutex_lock(&decdList->mutex);
    decdList->ending ++;
    pthread_cond_signal(&decdList->rCond);
    pthread_mutex_unlock(&decdList->mutex);
}