/*
 * @Author: Cai Deng
 * @Date: 2020-11-05 09:12:19
 * @LastEditors: Cai Deng
 * @LastEditTime: 2022-07-18 14:28:23
 * @Description: 
 */
#include "idelta.h"
#include "xdelta/xdelta3.h"
#include "adler32.h"

extern uint64_t out_table_i[256], mod_table_i[256];
#ifdef DEBUG_2
extern uint64_t sim_counter[20];
extern pthread_mutex_t sim_counter_mutex;
#endif
extern uint8_t data_type;
#ifdef PART_TIME
extern double dedup_time;
extern pthread_mutex_t dedup_time_mutex;
#endif


#ifdef THREAD_OPTI
static void* get_instructions_thread(void *parameter)
{
    void        **arg   =   (void**)parameter;
    GArray      *cpx    =   (GArray*)arg[0],
                *cpy    =   (GArray*)arg[1],
                *cpl    =   (GArray*)arg[2],
                *inl    =   (GArray*)arg[3];
    GPtrArray   *inp    =   (GPtrArray*)arg[4];
    JBLOCKARRAY tar_jbarray =   (JBLOCKARRAY)arg[5],
                base_jbarray=   (JBLOCKARRAY)arg[6];
    uint64_t    tar_width   = (uint64_t)arg[7],
                base_width  = (uint64_t)arg[8],
                tar_from    = (uint64_t)arg[9],
                tar_to      = (uint64_t)arg[10],
                base_height = (uint64_t)arg[12];
    GHashTable  *subBlockTab= (GHashTable*)arg[11];
    #ifdef DEBUG_2
    uint64_t    *simCounter = (uint64_t*)arg[13];
    #endif
    #ifdef FIX_OPTI
    uint64_t    fix_flag    =   (uint64_t)arg[14];
    
    if(fix_flag)
    {
        for(COPY_Y row=tar_from; row<=tar_to; row++)
        {
            COPY_X   column = 0;
            INSERT_L in_len = 0;
            while(column <= tar_width-LBS)
            {
                COPY_X x, xmax;
                COPY_Y y, ymax;
                COPY_L len = 0, lenmax = 0;
                #ifndef DC_HASH
                uint8_t *data_ptr = (uint8_t*)(tar_jbarray[row][column]);
                #endif
                #ifdef USE_RABIN
                #ifndef DC_HASH
                uint32_t hash = adler32(1, data_ptr, sizeof(JBLOCK));
                #else
                uint16_t *data_ptr = (uint16_t*)(tar_jbarray[row][column]);
                uint32_t hash = (data_ptr[0]<<24)|(data_ptr[1]&0xff<<16)|(data_ptr[8]&0xff<<8)|(data_ptr[9]&0xff);
                #endif
                digest_list *list = (digest_list*)g_hash_table_lookup(subBlockTab, &hash);

                #else
                uint64_t *hash = (uint64_t*)g_malloc0(sizeof(uint64_t));
                // gear_slide_a_block(hash, data_ptr);
                for(int i=1; i<128; i+=2)
                {
                    *hash <<= 1;
                    if(data_ptr[i]&1) *hash |= 1;
                }
                digest_list *list = (digest_list*)g_hash_table_lookup(subBlockTab, hash);
                free(hash);
                #endif

                if(list)
                {
                    digest_ptr d_item = list->head;
                    while(d_item)
                    {
                        x = d_item->x;
                        y = d_item->y;
                        len = 0;
                        for(int j=x,k=column; j<=base_width-LBS && k<=tar_width-LBS; j++,k++,len++)
                        {
                            if(memcmp(base_jbarray[y][j], tar_jbarray[row][k], sizeof(JBLOCK)))
                                break;
                        }
                        if(len > lenmax)
                        {
                            lenmax = len;
                            xmax = x;
                            ymax = y;
                        }
                        #ifdef TURN_ON_DELTA_OPTI
                        /*  The same position is most likely to be the best 
                        *  matching position.  */
                        if(x==column && y==row && lenmax>0)
                            break;
                        /*------------------------------------------------*/
                        #endif
                        d_item = d_item->next;
                    }
                }
                if(lenmax)
                {
                    g_array_append_val(cpx,xmax);
                    g_array_append_val(cpy,ymax);
                    g_array_append_val(cpl,lenmax);
                    g_array_append_val(inl,in_len);
                    in_len = 0;
                    column += lenmax;
                    #ifdef DEBUG_2
                    *simCounter += lenmax;
                    #endif
                }
                else 
                {
                    g_ptr_array_add(inp, data_ptr);
                    in_len ++;
                    column ++;
                }
            }
            if(in_len)
                g_array_append_val(inl,in_len);
        }
    }
    else
    {
    #endif
        for(COPY_Y row=tar_from; row<=tar_to; row++)
        {
            COPY_X   column = 0;
            INSERT_L in_len = 0;
            while(column <= tar_width-LBS)
            {
                COPY_L len = 0;
                uint8_t *data_ptr = (uint8_t*)(tar_jbarray[row][column]);
                
                for(int i=column; i<=base_width-LBS && i<=tar_width-LBS && row<=base_height-LBS; i++,len++)
                {
                    if(memcmp(base_jbarray[row][i],tar_jbarray[row][i],sizeof(JBLOCK)))
                        break;
                }
                if(len)
                {
                    g_array_append_val(cpx,column);
                    g_array_append_val(cpy,row);
                    g_array_append_val(cpl,len);
                    g_array_append_val(inl,in_len);
                    in_len = 0;
                    column += abs(len);
                    #ifdef DEBUG_2
                    *simCounter += abs(len);
                    #endif
                }
                else 
                {
                    g_ptr_array_add(inp, data_ptr);
                    in_len ++;
                    column ++;
                }
            }
            if(in_len)
                g_array_append_val(inl,in_len);
        }
    #ifdef FIX_OPTI
    }
    #endif
}

// static void* get_instructions_thread(void *parameter)
// {
//     void        **arg   =   (void**)parameter;
//     GArray      *cpx    =   (GArray*)arg[0],
//                 *cpy    =   (GArray*)arg[1],
//                 *cpl    =   (GArray*)arg[2],
//                 *inl    =   (GArray*)arg[3];
//     GPtrArray   *inp    =   (GPtrArray*)arg[4];
//     JBLOCKARRAY tar_jbarray = (JBLOCKARRAY)arg[5],
//                 base_jbarray= (JBLOCKARRAY)arg[6];
//     uint64_t    tar_width   = (uint64_t)arg[7],
//                 base_width  = (uint64_t)arg[8],
//                 tar_from    = (uint64_t)arg[9],
//                 tar_to      = (uint64_t)arg[10],
//                 base_height = (uint64_t)arg[12];
//     GHashTable  *subBlockTab= (GHashTable*)arg[11];

//     COPY_Y  row =   tar_from;
//     COPY_X  column = 0;
//     INSERT_L in_len = 0;
//     COPY_X x, xmax;
//     COPY_Y y, ymax;
//     while(row <= tar_to)
//     {
//         COPY_Y len = 0, lenmax = 0;
//         uint8_t *data_ptr = (uint8_t*)(tar_jbarray[row][column]);
//         #ifdef USE_RABIN
//         // struct rabin_t *hash = rabin_init(sizeof(JBLOCK), out_table_i, mod_table_i);
//         // rabin_slide_a_block(hash, data_ptr, sizeof(JBLOCK), out_table_i, mod_table_i);
//         // digest_list *list = (digest_list*)g_hash_table_lookup(subBlockTab, &(hash->digest));
//         // rabin_free(hash);
        
//         uint32_t hash = adler32(1, data_ptr, sizeof(JBLOCK));
//         digest_list *list = (digest_list*)g_hash_table_lookup(subBlockTab, &hash);
//         #else
//         codes needed 
//         #endif

//         if(list)
//         {
//             digest_ptr d_item = list->head;
//             while(d_item)
//             {
//                 x = d_item->x;
//                 y = d_item->y;
//                 len = 0;
//                 int j = x, k = y, l = column, m = row;
//                 while(k<base_height && m<=tar_to)
//                 {
//                     if(memcmp(base_jbarray[k][j], tar_jbarray[m][l],sizeof(JBLOCK)))
//                         break;
//                     if(++j==base_width)
//                     {
//                         j = 0;
//                         k++;
//                     }
//                     if(++l==tar_width)
//                     {
//                         l = 0;
//                         m++;
//                     }
//                     len++;
//                 }
//                 if(len > lenmax)
//                 {
//                     lenmax = len;
//                     ymax = y;
//                     xmax = x;
//                 }
//                 #ifdef TURN_ON_DELTA_OPTI
//                 /*  The same position is most likely to be the best 
//                 *  matching position.  */
//                 if(x==column && y==row && lenmax>0)
//                     break;
//                 /*------------------------------------------------*/
//                 #endif
//                 d_item = d_item->next;
//             }
//         }
//         if(lenmax)
//         {
//             g_array_append_val(cpx,xmax);
//             g_array_append_val(cpy,ymax);
//             g_array_append_val(cpl,lenmax);
//             g_array_append_val(inl,in_len);
//             in_len = 0;
//             column += lenmax;
//             row += column/tar_width;
//             column = column%tar_width;
//         }
//         else
//         {
//             g_ptr_array_add(inp, data_ptr);
//             in_len ++;
//             if(++column==tar_width)
//             {
//                 column = 0;
//                 row++;
//             }
//         }
//     }
//     if(in_len)
//         g_array_append_val(inl,in_len);
// }

static void get_instructions(dedupResPtr dedupPtr, jpeg_coe_ptr base, jpeg_coe_ptr target
    #ifdef THREAD_OPTI
    , GHashTable **subBlockTab
    #endif
    #ifdef FIX_OPTI
    , uint64_t fix_flag
    #endif
)
{
    #ifndef THREAD_OPTI
    GHashTable  **subBlockTab = create_block_index(base);
    #endif

    GArray      *cpx[3*SUB_THREAD_NUM], *cpy[3*SUB_THREAD_NUM], *cpl[3*SUB_THREAD_NUM], 
                *inl[3*SUB_THREAD_NUM], *inp[3*SUB_THREAD_NUM];
    uint64_t    tar_width[3], tar_height[3], base_width[3], base_height[3];

    for(int i=0; i<3*SUB_THREAD_NUM; i++)
    {
        cpx[i]  =   g_array_new(FALSE, FALSE, sizeof(COPY_X));
        cpy[i]  =   g_array_new(FALSE, FALSE, sizeof(COPY_Y));
        cpl[i]  =   g_array_new(FALSE, FALSE, sizeof(COPY_L));
        inl[i]  =   g_array_new(FALSE, FALSE, sizeof(INSERT_L));
        inp[i]  =   g_array_new(FALSE, FALSE, sizeof(uint8_t*));
    }

    for(int i=0; i<3; i++)
    {
        tar_width[i]    =   target->imgSize[i*2];
        tar_height[i]   =   target->imgSize[i*2+1];
        base_width[i]   =   base->imgSize[i*2];
        base_height[i]  =   base->imgSize[i*2+1];
    }

    uint8_t     *tar_p[3], *base_p[3];
    tar_p[0]    =   target->data;
    base_p[0]   =   base->data;
    for(int i=1; i<3; i++)
    {
        tar_p[i]    =   tar_p[i-1] + tar_width[i-1]*tar_height[i-1]*sizeof(JBLOCK);
        base_p[i]   =   base_p[i-1] + base_width[i-1]*base_height[i-1]*sizeof(JBLOCK);
    }

    JBLOCKARRAY tar_jbarray[3], base_jbarray[3];
    JBLOCKROW   tarrow0[tar_height[0]], baserow0[base_height[0]];
    for(int j=0; j<tar_height[0]; j++, tar_p[0]+=sizeof(JBLOCK)*tar_width[0])
        tarrow0[j]   =   (JBLOCKROW)tar_p[0];
    for(int j=0; j<base_height[0]; j++, base_p[0]+=sizeof(JBLOCK)*base_width[0])
        baserow0[j]  =   (JBLOCKROW)base_p[0];
    tar_jbarray[0]  =   tarrow0;
    base_jbarray[0] =   baserow0;
    JBLOCKROW   tarrow1[tar_height[1]], baserow1[base_height[1]];
    for(int j=0; j<tar_height[1]; j++, tar_p[1]+=sizeof(JBLOCK)*tar_width[1])
        tarrow1[j]   =   (JBLOCKROW)tar_p[1];
    for(int j=0; j<base_height[1]; j++, base_p[1]+=sizeof(JBLOCK)*base_width[1])
        baserow1[j]  =   (JBLOCKROW)base_p[1];
    tar_jbarray[1]  =   tarrow1;
    base_jbarray[1] =   baserow1;
    JBLOCKROW   tarrow2[tar_height[2]], baserow2[base_height[2]];
    for(int j=0; j<tar_height[2]; j++, tar_p[2]+=sizeof(JBLOCK)*tar_width[2])
        tarrow2[j]   =   (JBLOCKROW)tar_p[2];
    for(int j=0; j<base_height[2]; j++, base_p[2]+=sizeof(JBLOCK)*base_width[2])
        baserow2[j]  =   (JBLOCKROW)base_p[2];
    tar_jbarray[2]  =   tarrow2;
    base_jbarray[2] =   baserow2;

    pthread_t   p_id[3*SUB_THREAD_NUM];
    void        **arg[3*SUB_THREAD_NUM];
    for(int i=0; i<3; i++)
    {
        uint64_t    ave_height  =   tar_height[i]/SUB_THREAD_NUM;
        for(int j=0; j<SUB_THREAD_NUM; j++)
        {
            uint64_t    id  =   i*SUB_THREAD_NUM+j;
            arg[id]     =   (void**)malloc(sizeof(void*)*15);
            arg[id][0]  =   cpx[id];
            arg[id][1]  =   cpy[id];
            arg[id][2]  =   cpl[id];
            arg[id][3]  =   inl[id];
            arg[id][4]  =   inp[id];
            arg[id][5]  =   tar_jbarray[i];
            arg[id][6]  =   base_jbarray[i];
            arg[id][7]  =   (void*)tar_width[i];
            arg[id][8]  =   (void*)base_width[i];
            arg[id][9]  =   (void*)(uint64_t)(j*ave_height);
            arg[id][10] =   (void*)(uint64_t)((j==SUB_THREAD_NUM-1) ? (tar_height[i]-LBS) : ((j+1)*ave_height-1));
            arg[id][11] =   subBlockTab[i];
            arg[id][12] =   (void*)base_height[i];
            #ifdef DEBUG_2
            arg[id][13] =   (void*)g_malloc0(sizeof(uint64_t));
            #endif
            #ifdef FIX_OPTI
            arg[id][14] =   (void*)(uint64_t)fix_flag;
            #endif

            pthread_create(&p_id[id], NULL, get_instructions_thread, (void*)arg[id]);
        }
    }

    #ifdef DEBUG_2
    uint64_t simCounter = 0;
    #endif
    for(int i=0; i<3*SUB_THREAD_NUM; i++)
    {
        pthread_join(p_id[i], NULL);
        #ifdef DEBUG_2
        simCounter += *((uint64_t*)arg[i][13]);
        free(arg[i][13]);
        #endif
        free(arg[i]);
    }
    #ifdef DEBUG_2
    uint64_t index = ((double)simCounter/(tar_width[0]*tar_height[0]+tar_width[1]*tar_height[1]+tar_width[2]*tar_height[2]))*20;
    if(index==20) index = 19;
    pthread_mutex_lock(&sim_counter_mutex);
    sim_counter[index] ++;
    pthread_mutex_unlock(&sim_counter_mutex);
    #endif

    dedupPtr->y_counter =   0;
    dedupPtr->u_counter =   0;
    dedupPtr->v_counter =   0;
    #ifdef JPEG_SEPA_COMP
    dedupPtr->p_counter[0]  =   0;
    dedupPtr->p_counter[1]  =   0;
    dedupPtr->p_counter[2]  =   0;
    #endif
    for(int i=0; i<SUB_THREAD_NUM; i++)
    {
        dedupPtr->y_counter +=   inl[0*SUB_THREAD_NUM+i]->len;
        dedupPtr->u_counter +=   inl[1*SUB_THREAD_NUM+i]->len;
        dedupPtr->v_counter +=   inl[2*SUB_THREAD_NUM+i]->len;
        #ifdef JPEG_SEPA_COMP
        dedupPtr->p_counter[0]  +=  inp[0*SUB_THREAD_NUM+i]->len;
        dedupPtr->p_counter[1]  +=  inp[1*SUB_THREAD_NUM+i]->len;
        dedupPtr->p_counter[2]  +=  inp[2*SUB_THREAD_NUM+i]->len;
        #endif
    }

    for(int i=1; i<3*SUB_THREAD_NUM; i++)
    {
        g_array_append_vals(cpx[0], cpx[i]->data, cpx[i]->len);
        g_array_free(cpx[i], TRUE);
        g_array_append_vals(cpy[0], cpy[i]->data, cpy[i]->len);
        g_array_free(cpy[i], TRUE);
        g_array_append_vals(cpl[0], cpl[i]->data, cpl[i]->len);
        g_array_free(cpl[i], TRUE);
        g_array_append_vals(inl[0], inl[i]->data, inl[i]->len);
        g_array_free(inl[i], TRUE);
        g_array_append_vals(inp[0], inp[i]->data, inp[i]->len);
        g_array_free(inp[i], TRUE);
    }

    for(int i=0; i<3; i++)
        g_hash_table_destroy(subBlockTab[i]);
    free(subBlockTab);

    dedupPtr->copy_x    =   cpx[0];
    dedupPtr->copy_y    =   cpy[0];
    dedupPtr->copy_l    =   cpl[0];
    dedupPtr->insert_l  =   inl[0];
    dedupPtr->insert_p  =   inp[0];
}
#endif

static dedupResPtr dedup_a_single_img(detectionDataPtr detectPtr)
{
    imagePtr        baseImage   =   (imagePtr)detectPtr->base->data,
                    targetImage =   (imagePtr)detectPtr->target->data;
    jpeg_coe_ptr    baseCoe     =   baseImage->decdData->targetInfo->coe,
                    targetCoe   =   targetImage->decdData->targetInfo->coe;
    dedupResPtr     dedupPtr    =   (dedupResPtr)malloc(sizeof(dedupResNode));
    #ifdef HEADER_DELTA
    uint64_t    ava_oup_size    =   targetCoe->headerSize;
    uint8_t     *buffer =   (uint8_t*)malloc(ava_oup_size);
    uint64_t    delSize;
    while(
        ENOSPC == xd3_encode_memory(targetCoe->header,
                    targetCoe->headerSize,
                    baseCoe->header,
                    baseCoe->headerSize,
                    buffer,
                    &delSize,
                    ava_oup_size,
                    1)
    )
    {
        ava_oup_size <<= 1;
        free(buffer);
        buffer  =   (uint8_t*)malloc(ava_oup_size);
    }
    dedupPtr->header    =   buffer;
    dedupPtr->headerSize=   delSize;
    #else
    dedupPtr->header    =   targetCoe->header;
    dedupPtr->headerSize=   targetCoe->headerSize;
    #endif
    dedupPtr->baseName  =   baseImage->decdData->rawData->name;
    dedupPtr->name      =   targetImage->decdData->rawData->name;
    dedupPtr->imgSize[0]=   targetCoe->imgSize[0];
    dedupPtr->imgSize[1]=   targetCoe->imgSize[1];
    dedupPtr->imgSize[2]=   targetCoe->imgSize[2];
    dedupPtr->imgSize[3]=   targetCoe->imgSize[3];
    dedupPtr->ffxx      =   targetImage->decdData->targetInfo->ffxx;
    dedupPtr->xx        =   targetImage->decdData->targetInfo->xx;
    dedupPtr->node      =   detectPtr->target;
    #ifdef ORIGINAL_HUFF
    memcpy(dedupPtr->hufTab, targetCoe->hufTab, sizeof(targetCoe->hufTab));
    #endif

    get_instructions(dedupPtr, baseCoe, targetCoe
        #ifdef THREAD_OPTI
        , detectPtr->subBlockTab
        #endif
        #ifdef FIX_OPTI
        , detectPtr->fix_flag
        #endif
    );

    if((dedupPtr->copy_l == NULL) && (dedupPtr->insert_p == NULL))
    {
        free(dedupPtr->header);
        free(dedupPtr);
        return NULL;
    }

    dedupPtr->mem_size  =   dedupPtr->copy_x->len*sizeof(COPY_X) + dedupPtr->copy_y->len*sizeof(COPY_Y) +
                            dedupPtr->copy_l->len*sizeof(COPY_L) + dedupPtr->insert_l->len*sizeof(INSERT_L) +
                            dedupPtr->insert_p->len*sizeof(uint8_t*) + sizeof(dedupResNode)
                            #ifdef HEADER_DELTA
                            + dedupPtr->headerSize
                            #endif
                            ;

    return dedupPtr;
}

void* dedup_thread(void *parameter)
{
    void        **arg       =   (void**)parameter;
    List        detectList  =   (List)arg[0];
    List        dedupList   =   (List)arg[1];
    char        *outPath    =   (char*)arg[2];
    detectionDataPtr    detectPtr;
    dedupResPtr dedupPtr;
    #ifdef PART_TIME
    GTimer      *timer      =   g_timer_new();
    #endif
    uint64_t    *deltaedSize=   (uint64_t*)g_malloc0(sizeof(uint64_t)*3);

    while(1)
    {
        pthread_mutex_lock(&detectList->mutex);
        while(detectList->counter == 0)
        {
            if(detectList->ending) goto ESCAPE_LOOP;
            pthread_cond_wait(&detectList->rCond, &detectList->mutex);
        }
        detectPtr   =   detectList->head;
        detectList->head    =   detectPtr->next;
        detectList->size    +=  detectPtr->mem_size;
        detectList->counter --;
        pthread_cond_signal(&detectList->wCond);
        pthread_mutex_unlock(&detectList->mutex);

        if(detectPtr)
        {
            decodedDataPtr decdPtr  =   ((imagePtr)(detectPtr->target->data))->decdData;
            deltaedSize[0]  +=  decdPtr->rawData->size;
            deltaedSize[1]  +=  (decdPtr->targetInfo->coe->imgSize[0]*decdPtr->targetInfo->coe->imgSize[1]
                            + decdPtr->targetInfo->coe->imgSize[2]*decdPtr->targetInfo->coe->imgSize[3]
                            + decdPtr->targetInfo->coe->imgSize[4]*decdPtr->targetInfo->coe->imgSize[5]) * sizeof(JBLOCK);

            #ifdef PART_TIME
            g_timer_start(timer);
            #endif

            dedupPtr    =   dedup_a_single_img(detectPtr);
            pthread_mutex_lock(&detectPtr->base->mutex);
            detectPtr->base->link   --;
            pthread_mutex_unlock(&detectPtr->base->mutex);

            #ifdef PART_TIME
            pthread_mutex_lock(&dedup_time_mutex);
            dedup_time  +=  g_timer_elapsed(timer, NULL);
            pthread_mutex_unlock(&dedup_time_mutex);
            #endif

            if(!dedupPtr)
            {
                deltaedSize[2]  +=  decdPtr->rawData->size;
                #ifndef DO_NOT_WRITE
                char    outFilePath[MAX_PATH_LEN];
                PUT_3_STRS_TOGETHER(outFilePath, outPath, "/", decdPtr->rawData->name);
                FILE    *outFp  =   fopen(outFilePath, "wb");
                fwrite(decdPtr->rawData->data, 1, decdPtr->rawData->size, outFp);
                // fflush(outFp);
                // fsync(fileno(outFp));
                fclose(outFp);
                #endif
            }
            else
            {
                dedupPtr->next  =   NULL;
                pthread_mutex_lock(&dedupList->mutex);
                while(dedupList->size < dedupPtr->mem_size)
                    pthread_cond_wait(&dedupList->wCond, &dedupList->mutex);
                if(dedupList->counter)
                    ((dedupResPtr)dedupList->tail)->next    =   dedupPtr;
                else 
                    dedupList->head =   dedupPtr;
                dedupList->tail =   dedupPtr;
                dedupList->counter ++;
                dedupList->size -=  dedupPtr->mem_size;
                pthread_cond_signal(&dedupList->rCond);
                pthread_mutex_unlock(&dedupList->mutex);
            }

            free(detectPtr);
        }
    }

    ESCAPE_LOOP:
    pthread_mutex_unlock(&detectList->mutex);

    #ifdef PART_TIME
    g_timer_destroy(timer);
    #endif

    pthread_mutex_lock(&dedupList->mutex);
    dedupList->ending   ++;
    for(int i=0; i<REJPEG_THREAD_NUM; i++)
        pthread_cond_signal(&dedupList->rCond);
    pthread_mutex_unlock(&dedupList->mutex);

    return (void*)deltaedSize;
}

static dedupResPtr tra_dedup_a_single_img(detectionDataPtr detectPtr)
{
    imagePtr        baseImage   =   (imagePtr)detectPtr->base->data,
                    targetImage =   (imagePtr)detectPtr->target->data;
    jpeg_coe_ptr    baseCoe     =   baseImage->decdData->targetInfo->coe,
                    targetCoe   =   targetImage->decdData->targetInfo->coe;
    dedupResPtr     dedupPtr    =   (dedupResPtr)malloc(sizeof(dedupResNode));

    if(data_type == DECODED)
    {
        #ifdef HEADER_DELTA
        uint64_t    ava_oup_size    =   targetCoe->headerSize;
        uint8_t     *buffer =   (uint8_t*)malloc(ava_oup_size);
        uint64_t    delSize;
        while(
            ENOSPC == xd3_encode_memory(targetCoe->header,
                        targetCoe->headerSize,
                        baseCoe->header,
                        baseCoe->headerSize,
                        buffer,
                        &delSize,
                        ava_oup_size,
                        1)
        )
        {
            ava_oup_size <<= 1;
            free(buffer);
            buffer  =   (uint8_t*)malloc(ava_oup_size);
        }
        dedupPtr->header    =   buffer;
        dedupPtr->headerSize=   delSize;
        #else
        dedupPtr->header    =   targetCoe->header;
        dedupPtr->headerSize=   targetCoe->headerSize;
        #endif
    }
    else
    {
        dedupPtr->header = NULL;
        dedupPtr->headerSize = 0;
    }

    dedupPtr->baseName  =   baseImage->decdData->rawData->name;
    dedupPtr->name      =   targetImage->decdData->rawData->name;
    dedupPtr->imgSize[0]=   targetCoe->imgSize[0];
    dedupPtr->imgSize[1]=   targetCoe->imgSize[1];
    dedupPtr->imgSize[2]=   targetCoe->imgSize[2];
    dedupPtr->imgSize[3]=   targetCoe->imgSize[3];
    dedupPtr->ffxx      =   targetImage->decdData->targetInfo->ffxx;
    dedupPtr->xx        =   targetImage->decdData->targetInfo->xx;
    dedupPtr->node      =   detectPtr->target;

    uint64_t        target_size, source_size, avadata_size;
    if(data_type == DECODED)
    {
        target_size =   (targetCoe->imgSize[0]*targetCoe->imgSize[1]+
                        targetCoe->imgSize[2]*targetCoe->imgSize[3]*2)*sizeof(JBLOCK);
        source_size =   (baseCoe->imgSize[0]*baseCoe->imgSize[1]+
                        baseCoe->imgSize[2]*baseCoe->imgSize[3]*2)*sizeof(JBLOCK);
        avadata_size=   target_size;
    }
    else
    {
        target_size =   targetImage->decdData->rawData->size;
        source_size =   baseImage->decdData->rawData->size;
        avadata_size=   target_size;
    }
    uint8_t         *dataBuffer =   (uint8_t*)malloc(avadata_size);
    uint64_t        dataDelSize;
    if(data_type == DECODED)
    {
        while(
            ENOSPC == xd3_encode_memory(
                        targetCoe->data,
                        target_size,
                        baseCoe->data,
                        source_size,
                        dataBuffer,
                        &dataDelSize,
                        avadata_size,
                        1)
        )
        {
            avadata_size <<= 1;
            free(dataBuffer);
            dataBuffer  =   (uint8_t*)malloc(avadata_size);
        }
    }
    else
    {
        while(
            ENOSPC == xd3_encode_memory(
                        targetImage->decdData->rawData->data,
                        target_size,
                        baseImage->decdData->rawData->data,
                        source_size,
                        dataBuffer,
                        &dataDelSize,
                        avadata_size,
                        1)
        )
        {
            avadata_size <<= 1;
            free(dataBuffer);
            dataBuffer  =   (uint8_t*)malloc(avadata_size);
        }
    }
    dedupPtr->insert_l  =   (GArray*)dataBuffer;
    dedupPtr->v_counter =   (uint32_t)dataDelSize;
    dedupPtr->insert_p  =   g_array_new(FALSE, FALSE, 1);

    dedupPtr->mem_size  =   dataDelSize + sizeof(dedupResNode)
                            #ifdef HEADER_DELTA
                            + dedupPtr->headerSize
                            #endif
                            ;

    return dedupPtr;
}

void* tra_dedup_thread(void *parameter)
{
    void        **arg       =   (void**)parameter;
    List        detectList  =   (List)arg[0];
    List        dedupList   =   (List)arg[1];
    detectionDataPtr    detectPtr;
    dedupResPtr dedupPtr;
    #ifdef PART_TIME
    GTimer      *timer      =   g_timer_new();
    #endif
    uint64_t    *deltaedSize=   (uint64_t*)g_malloc0(sizeof(uint64_t)*2);

    while(1)
    {
        pthread_mutex_lock(&detectList->mutex);
        while(detectList->counter == 0)
        {
            if(detectList->ending) goto ESCAPE_LOOP;
            pthread_cond_wait(&detectList->rCond, &detectList->mutex);
        }
        detectPtr   =   detectList->head;
        detectList->head    =   detectPtr->next;
        detectList->size    +=  detectPtr->mem_size;
        detectList->counter --;
        pthread_cond_signal(&detectList->wCond);
        pthread_mutex_unlock(&detectList->mutex);

        if(detectPtr)
        {
            decodedDataPtr decdPtr  =   ((imagePtr)(detectPtr->target->data))->decdData;
            deltaedSize[0]  +=  decdPtr->rawData->size;
            deltaedSize[1]  +=  (decdPtr->targetInfo->coe->imgSize[0]*decdPtr->targetInfo->coe->imgSize[1]
                            + decdPtr->targetInfo->coe->imgSize[2]*decdPtr->targetInfo->coe->imgSize[3]
                            + decdPtr->targetInfo->coe->imgSize[4]*decdPtr->targetInfo->coe->imgSize[5]) * sizeof(JBLOCK);
                            
            #ifdef PART_TIME
            g_timer_start(timer);
            #endif

            dedupPtr    =   tra_dedup_a_single_img(detectPtr);
            pthread_mutex_lock(&detectPtr->base->mutex);
            detectPtr->base->link   --;
            pthread_mutex_unlock(&detectPtr->base->mutex);

            #ifdef PART_TIME
            pthread_mutex_lock(&dedup_time_mutex);
            dedup_time  +=  g_timer_elapsed(timer, NULL);
            pthread_mutex_unlock(&dedup_time_mutex);
            #endif

            dedupPtr->next  =   NULL;
            pthread_mutex_lock(&dedupList->mutex);
            while(dedupList->size < dedupPtr->mem_size)
                pthread_cond_wait(&dedupList->wCond, &dedupList->mutex);
            if(dedupList->counter)
                ((dedupResPtr)dedupList->tail)->next    =   dedupPtr;
            else 
                dedupList->head =   dedupPtr;
            dedupList->tail =   dedupPtr;
            dedupList->counter ++;
            dedupList->size -=  dedupPtr->mem_size;
            pthread_cond_signal(&dedupList->rCond);
            pthread_mutex_unlock(&dedupList->mutex);

            free(detectPtr);
        }
    }

    ESCAPE_LOOP:
    pthread_mutex_unlock(&detectList->mutex);

    #ifdef PART_TIME
    g_timer_destroy(timer);
    #endif

    pthread_mutex_lock(&dedupList->mutex);
    dedupList->ending   ++;
    pthread_cond_signal(&dedupList->rCond);
    pthread_mutex_unlock(&dedupList->mutex);

    return (void*)deltaedSize;
}

/*---------------------------------------------------------------------------------------*/

static de_dedupPtr de_dedup_a_single_img(de_readPtr decodePtr, jpeg_coe_ptr base)
{
    uint32_t    totalSize   =   decodePtr->sizes[0]*(decodePtr->sizes[1])+decodePtr->sizes[2]*decodePtr->sizes[3]*2;
    uint8_t     *dataPtr    =   (uint8_t*)malloc(sizeof(JBLOCK)*totalSize);
    INSERT_L    *insert_l   =   (INSERT_L*)decodePtr->in_l;
    COPY_X      *copy_x     =   (COPY_X*)decodePtr->x;
    COPY_Y      *copy_y     =   (COPY_Y*)decodePtr->y;
    COPY_L      *copy_l     =   (COPY_L*)decodePtr->cp_l;
    uint8_t     *inp        =   decodePtr->in_d;
    uint8_t     *base_p     =   base->data;
    jvirt_barray_ptr   *coe =   (jvirt_barray_ptr*)malloc(sizeof(jvirt_barray_ptr)*3);
    int         i, j, k;
    COPY_X      tar_width, x;
    COPY_Y      tar_height, y;

    de_dedupPtr     dedupPtr    =   (de_dedupPtr)malloc(sizeof(de_dedupNode));
    dedupPtr->mem_size  =   sizeof(de_dedupNode) + sizeof(JBLOCK)*totalSize;
   
    for(i=0;i<3;i++)
        coe[i] = (jvirt_barray_ptr)g_malloc0(sizeof(struct jvirt_barray_control));
    dedupPtr->mem_size  +=  sizeof(struct jvirt_barray_control)*3;

    for(i=0; i<3; i++)
    {
        if(i == 0)
        {
            tar_width   =   decodePtr->sizes[0];
            tar_height  =   decodePtr->sizes[1];
        }
        else 
        {
            tar_width   =   decodePtr->sizes[2];
            tar_height  =   decodePtr->sizes[3];
        }
        JBLOCKROW   *jbrow  =   (JBLOCKROW*)malloc(sizeof(JBLOCKROW)*tar_height);
        dedupPtr->mem_size  +=  sizeof(JBLOCKROW)*tar_height;
        uint8_t     *coePtr =   dataPtr;
        for(j=0; j<tar_height; j++, coePtr+=tar_width*sizeof(JBLOCK))
            jbrow[j]    =   (JBLOCKROW)coePtr;
        coe[i]->mem_buffer      =   (JBLOCKARRAY)jbrow;
        coe[i]->blocksperrow    =   tar_width;
        coe[i]->rows_in_array   =   tar_height;
        coe[i]->maxaccess       =   2;
        coe[i]->rows_in_mem     =   coe[i]->rows_in_array;
        coe[i]->rowsperchunk    =   coe[i]->rows_in_array;
        coe[i]->first_undef_row =   coe[i]->rows_in_array;
        coe[i]->pre_zero        =   1;
        coe[i]->dirty           =   1;

        uint32_t    base_w  =   base->imgSize[i*2],
                    base_h  =   base->imgSize[i*2+1];
        JBLOCKROW   baserow[base_h];
        for(j=0; j<base_h; j++, base_p+=sizeof(JBLOCK)*base_w)
            baserow[j] = (JBLOCKROW)base_p;
        JBLOCKARRAY jbarray =   baserow;
        uint32_t    ready   =   0;

        for(j=0; j<decodePtr->sizes[4+i]; j++)
        {
            INSERT_L    inlen   =   *insert_l++;
            if(inlen)
            {
                uint32_t    dataLen =   inlen * sizeof(JBLOCK);
                memcpy(dataPtr, inp, dataLen);
                dataPtr +=  dataLen;
                inp     +=  dataLen;
                ready   +=  inlen;
            }
            if(ready < tar_width)
            {
                y   =   *copy_y++;
                x   =   *copy_x++;
                COPY_L  cplen = *copy_l++;
                for(k=0; k<cplen; k++, dataPtr+=sizeof(JBLOCK))
                    memcpy(dataPtr, jbarray[y][x+k], sizeof(JBLOCK));
                ready   +=  cplen;
            }
            if(ready == tar_width)
                ready = 0;
        }
    }

    if(coe[0]->blocksperrow > coe[1]->blocksperrow)
    {
        // coe[i]->maxaccess       ++;
        if(coe[0]->blocksperrow & 1)
            coe[0]->blocksperrow ++;
    }
    if((coe[0]->rows_in_array > coe[1]->rows_in_array) && (coe[0]->rows_in_array & 1))
    {
        coe[0]->first_undef_row ++;
        coe[0]->rows_in_array ++;
        coe[0]->rows_in_mem ++;
        coe[0]->rowsperchunk ++;
    }
    coe[2]->next = coe[1];
    coe[1]->next = coe[0];
    coe[0]->next = NULL;

    jpeg_coe_ptr    content_p   =   (jpeg_coe_ptr)malloc(sizeof(jpeg_coe));
    content_p->data         =   (uint8_t*)coe[0]->mem_buffer[0];
    content_p->imgSize[0]   =   decodePtr->sizes[0];
    content_p->imgSize[1]   =   decodePtr->sizes[1];
    content_p->imgSize[2]   =   decodePtr->sizes[2];
    content_p->imgSize[3]   =   decodePtr->sizes[3];
    content_p->imgSize[4]   =   decodePtr->sizes[2];
    content_p->imgSize[5]   =   decodePtr->sizes[3];
    #ifdef HEADER_DELTA
    uint64_t    bufSize     =   base->headerSize;
    uint8_t     *buffer     =   (uint8_t*)malloc(1);
    uint64_t    resSize     =   0;
    do 
    {
        bufSize <<= 1;
        free(buffer);
        buffer  =   (uint8_t*)malloc(bufSize);
        xd3_decode_memory(decodePtr->header,
                        decodePtr->sizes[7],
                        base->header,
                        base->headerSize,
                        buffer,
                        &resSize,
                        bufSize,
                        1);
    }   while(resSize<=0);
    content_p->header       =   buffer;
    content_p->headerSize   =   resSize;
    dedupPtr->mem_size      +=  resSize;
    #else
    content_p->header       =   (uint8_t*)malloc(decodePtr->sizes[7]);
    memcpy(content_p->header, decodePtr->header, decodePtr->sizes[7]);
    content_p->headerSize   =   decodePtr->sizes[7];
    #endif
    dedupPtr->coe           =   coe;
    dedupPtr->content       =   content_p;
    dedupPtr->name          =   decodePtr->name;
    dedupPtr->oriPtr        =   decodePtr->basename_and_oriptr;
    dedupPtr->ffxx          =   decodePtr->ffxx;
    dedupPtr->xx            =   decodePtr->xx;

    return  dedupPtr;
}

// void* de_dedup_thread(void *parameter)
// {
//     void        **arg       =   (void**)parameter;
//     List        decodeList  =   (List)arg[0];
//     List        dedupList   =   (List)arg[1];
//     List        tabList     =   (List)arg[2];
//     GHashTable  *coeTable   =   (GHashTable*)tabList->head;
//     pthread_mutex_t mutex   =   tabList->mutex;
//     de_readPtr  decodePtr   =   decodeList->head, decodeTail    =   decodeList->tail, decodeTmp;
//     de_dedupPtr dedupPtr;
//     jpeg_coe_ptr    base;

//     while(decodePtr)
//     {
//         pthread_mutex_lock(&mutex);
//         base    =   g_hash_table_lookup(coeTable, decodePtr->basename_and_oriptr);
//         pthread_mutex_unlock(&mutex);
        
//         if(base)
//         {
//             dedupPtr    =   de_dedup_a_single_img(decodePtr, base);
//             pthread_mutex_lock(&mutex);
//             g_hash_table_insert(coeTable,dedupPtr->name,dedupPtr->content);
//             pthread_mutex_unlock(&mutex);

//             dedupPtr->next  =   NULL;
//             pthread_mutex_lock(&dedupList->mutex);
//             if(dedupList->counter)
//                 ((de_dedupPtr)dedupList->tail)->next    =   dedupPtr;
//             else 
//                 dedupList->head =   dedupPtr;
//             dedupList->tail =   dedupPtr;
//             dedupList->counter  ++;
//             pthread_cond_signal(&dedupList->rCond);
//             // if(dedupList->counter == OTHER_LIST_LEN)
//             //     pthread_cond_wait(&dedupList->wCond, &dedupList->mutex);
//             pthread_mutex_unlock(&dedupList->mutex);

//             decodeTmp   =   decodePtr->next;
//             #ifdef COMPRESS_DELTA_INS
//             if(decodePtr->flag & 0x08)    free(decodePtr->x);
//             if(decodePtr->flag & 0x04)    free(decodePtr->y);
//             if(decodePtr->flag & 0x02)    free(decodePtr->cp_l);
//             if(decodePtr->flag & 0x01)    free(decodePtr->in_l);
//             #endif
//             free(decodePtr->in_d);
//             free(decodePtr);
//             decodePtr   =   decodeTmp;
//             decodeList->counter --;
//         }
//         else 
//         {
//             // if(decodeList->counter == 1005)
//             // {
//             //     printf("%s----%s,%s\n",decodePtr->name,decodePtr->basename_and_oriptr);
//             //     break;
//             // }
//             decodeTail->next    =   decodePtr;
//             decodeTail  =   decodePtr;
//             decodePtr   =   decodePtr->next;
//             decodeTail->next    =   NULL;
//         }
//     }

//     pthread_mutex_lock(&dedupList->mutex);
//     dedupList->ending   =   1;
//     pthread_mutex_unlock(&dedupList->mutex);
//     pthread_cond_signal(&dedupList->rCond);
// }

static void free_buf_node(gpointer p)
{
    buf_node    *node   =   (buf_node*)p;
    de_node     *de_node_data   =   (de_node*)node->data;
    jpeg_coe_ptr    coe =   de_node_data->coe;
    free(coe->data);
    free(coe->header);
    free(coe);
    de_node_data->coe   =   NULL;
}

void* de_dedup_thread(void *parameter)
{
    void        **arg   =   (void**)parameter;
    List        decdList=   (List)arg[0];
    List        deupList=   (List)arg[1];
    pairPtr     pair;
    de_dedupPtr dedupPtr;
    pthread_mutex_t *tabMutex = (pthread_mutex_t*)arg[2];
    GHashTable  *imgTab =   (GHashTable*)arg[3];
    Buffer      *buf    =   (Buffer*)arg[4];
    #ifdef  PART_TIME
    GTimer      *timer  =   g_timer_new();
    #endif

    while(1)
    {
        pthread_mutex_lock(&decdList->mutex);
        while(decdList->counter == 0)
        {
            if(decdList->ending) goto ESCAPE_LOOP;
            pthread_cond_wait(&decdList->rCond, &decdList->mutex);
        }
        pair    =   decdList->head;
        decdList->head  =   pair->next;
        decdList->size  +=  pair->mem_size;
        decdList->counter   --;
        pthread_cond_signal(&decdList->wCond);
        pthread_mutex_unlock(&decdList->mutex);

        #ifdef  PART_TIME
        g_timer_start(timer);
        #endif

        dedupPtr    =   de_dedup_a_single_img(pair->target, ((de_node*)pair->base->data)->coe);
        dedupPtr->next  =   NULL;


        buf_node    *node = (buf_node*)malloc(sizeof(buf_node));
        de_node     *node_data = (de_node*)malloc(sizeof(de_node));
        node_data->name = malloc(strlen(pair->target->name)+1);
        node_data->folder = malloc(strlen(((de_node*)pair->base->data)->folder)+1);
        strcpy(node_data->name, pair->target->name);
        strcpy(node_data->folder, ((de_node*)pair->base->data)->folder);
        node_data->coe  = dedupPtr->content;
        node->size = dedupPtr->content->headerSize;
        for(int i=0; i<3; i++)
            node->size    +=  dedupPtr->content->imgSize[2*i]*dedupPtr->content->imgSize[2*i+1]*sizeof(JBLOCK);
        pthread_mutex_init(&node->mutex, NULL);
        node->data = node_data;
        node->link = 0;
        insert_to_buffer(node, buf, free_buf_node);
        pthread_mutex_lock(tabMutex);
        // if(!g_hash_table_lookup(imgTab, node_data->name))
            // printf("%s\n",node_data->name);
        g_hash_table_insert(imgTab, node_data->name, node);
        pthread_mutex_unlock(tabMutex);

        #ifdef PART_TIME
        pthread_mutex_lock(&dedup_time_mutex);
        dedup_time   +=  g_timer_elapsed(timer, NULL);
        pthread_mutex_unlock(&dedup_time_mutex);
        #endif

        pthread_mutex_lock(&deupList->mutex);
        while(deupList->size < dedupPtr->mem_size)
            pthread_cond_wait(&deupList->wCond, &deupList->mutex);
        if(deupList->counter)
            ((de_dedupPtr)deupList->tail)->next =   dedupPtr;
        else
            deupList->head  =   dedupPtr;
        deupList->tail  =   dedupPtr;
        deupList->counter   ++;
        deupList->size  -=  dedupPtr->mem_size;
        pthread_cond_signal(&deupList->rCond);
        pthread_mutex_unlock(&deupList->mutex);

        pthread_mutex_lock(&pair->base->mutex);
        pair->base->link    --;
        pthread_mutex_unlock(&pair->base->mutex);
        
        #ifdef COMPRESS_DELTA_INS
        if(pair->target->flag & 0x08)    free(pair->target->x);
        if(pair->target->flag & 0x04)    free(pair->target->y);
        if(pair->target->flag & 0x02)    free(pair->target->cp_l);
        if(pair->target->flag & 0x01)    free(pair->target->in_l);
        #endif
        free(pair->target->in_d);
        free(pair->target);

        free(pair);
    }

    ESCAPE_LOOP:
    pthread_mutex_unlock(&decdList->mutex);

    #ifdef PART_TIME
    g_timer_destroy(timer);
    #endif

    pthread_mutex_lock(&deupList->mutex);
    deupList->ending ++;
    pthread_cond_signal(&deupList->rCond);
    pthread_mutex_unlock(&deupList->mutex);
}