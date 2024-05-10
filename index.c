/*
 * @Author: Cai Deng
 * @Date: 2021-06-21 06:41:11
 * @LastEditors: Cai Deng dengcaidengcai@163.com
 * @LastEditTime: 2024-05-10 08:09:39
 * @Description: 
 */
#include "index.h"
#include "adler32.h"

#define LBS 1

extern uint64_t out_table_i[256], mod_table_i[256];
#ifdef PART_TIME
extern double index_time;
extern pthread_mutex_t index_time_mutex;
#endif


static void free_item(gpointer p)
{
    free(p);
}

static void free_digest_list(gpointer p)
{
    digest_list *list = (digest_list*)p;
    digest_ptr ptr = list->head, tmp;
    while(ptr)
    {
        tmp = ptr;
        ptr = ptr->next;
        free(tmp);
    }
    free(list);
}

static void *index_sub_thread(void *parameter)
{
    void    **arg   =   (void**)parameter;
    uint64_t    width = (uint64_t)arg[0];
    uint64_t    from = (uint64_t)arg[1];
    uint64_t    to = (uint64_t)arg[2];
    JBLOCKARRAY jbarray = (JBLOCKARRAY)arg[3];
    GHashTable  *subBlockTab    =   (GHashTable*)arg[4];
    pthread_mutex_t *mutex = (pthread_mutex_t*)arg[5];
    int j, row, column, k;
    uint8_t *block_p;
    #ifdef USE_RABIN
    // struct  rabin_t   *hash;
    // uint64_t    tmp;
    uint32_t    hash, tmp;
    #else
    uint64_t *hash;
    #endif
    uint64_t    flag; // begainning of the row?
    uint64_t    *size_part  =   (uint64_t*)g_malloc0(sizeof(uint64_t));

    for(row=from; row<=to; row++)
    {
        tmp = 0;
        flag = 1;
        #ifdef USE_RABIN
        // hash = rabin_init(sizeof(JBLOCK), out_table_i, mod_table_i);
        #else
        hash = (uint64_t*)g_malloc0(sizeof(uint64_t));
        #endif
        // for(column=0; column<LBS-1; column++)
        // {
        //     for(j=0; j<LBS; j++)
        //     {
        //         block_p = (uint8_t*)(jbarray[row+j][column]);
        //         #ifdef USE_RABIN
        //         rabin_slide_a_block(hash,block_p,sizeof(JBLOCK), out_table_i, mod_table_i);
        //         #else
        //         gear_slide_a_block(hash, block_p);
        //         #endif
        //     }
        // }
        for(column=0; column<=width-1; column++)
        {
            
            block_p = (uint8_t*)(jbarray[row+j][column]);
            #ifdef USE_RABIN
            // rabin_slide_a_block(hash,block_p,sizeof(JBLOCK), out_table_i, mod_table_i);
            #ifndef DC_HASH
            hash = adler32(1, block_p, sizeof(JBLOCK));
            #else
            uint16_t *data_ptr = (uint16_t*)block_p;
            hash = (data_ptr[0]<<24)|(data_ptr[1]&0xff<<16)|(data_ptr[8]&0xff<<8)|(data_ptr[9]&0xff);
            #endif
            #else
            // gear_slide_a_block(hash, block_p);
            *hash = 0;
            for(k=1; k<128; k+=2)
            {
                *hash <<= 1;
                if(block_p[k]&1)  *hash |= 1;
            }
            #endif
            
            #ifdef TUEN_ON_INDEX_OPTI
            #ifdef USE_RABIN
            // if(hash->digest != tmp || flag)
            if(hash != tmp || flag)
            #else
            if(*hash != tmp || flag)
            #endif
            #endif
            {
                digest_ptr value = (digest_ptr)malloc(sizeof(digest_node));
                value->x = column;
                value->y = row;
                value->next = NULL;
                pthread_mutex_lock(mutex);
                #ifdef USE_RABIN
                // digest_list *list = (digest_list*)g_hash_table_lookup(subBlockTab,&hash->digest);
                digest_list *list = (digest_list*)g_hash_table_lookup(subBlockTab,&hash);
                #else
                digest_list *list = (digest_list*)g_hash_table_lookup(subBlockTab,hash);
                #endif
                if(list)
                {
                    list->tail->next = value;
                    list->tail = value;
                }
                else 
                {
                    // uint64_t *key = (uint64_t*)malloc(sizeof(uint64_t));
                    uint32_t *key = (uint32_t*)malloc(sizeof(uint32_t));
                    #ifdef USE_RABIN
                    // *key = hash->digest;
                    *key = hash;
                    #else
                    *key = *hash;
                    #endif
                    list = (digest_list*)malloc(sizeof(digest_list));
                    list->tail = value;
                    list->head = value;
                    g_hash_table_insert(subBlockTab,key,list);
                    *size_part  +=  (sizeof(uint64_t) + sizeof(digest_list));
                }
                pthread_mutex_unlock(mutex);
                #ifdef USE_RABIN
                // tmp = hash->digest;
                tmp = hash;
                #else
                tmp = *hash;
                #endif
                *size_part  +=  sizeof(digest_node);
            }
            flag = 0;
        }
        #ifdef USE_RABIN
        // rabin_free(hash);
        #else 
        free(hash);
        #endif
    }

    return (void*)size_part;
}

static void *index_compute_thread(void *parameter)
{
    void     **arg   =   (void**)parameter;
    uint64_t width   =   (uint64_t)arg[0];
    uint64_t height  =   (uint64_t)arg[1];
    uint8_t  *ptr    =   (uint8_t*)arg[2];
    pthread_mutex_t  *mutex = (pthread_mutex_t*)arg[3];
    uint64_t *size   =   (uint64_t*)arg[4];
    // GHashTable *subBlockTab = g_hash_table_new_full(g_int64_hash,g_int64_equal,free_item,free_digest_list);
    GHashTable *subBlockTab = g_hash_table_new_full(g_int_hash,g_int_equal,free_item,free_digest_list);
    JBLOCKROW jbrow[height];
    for(int j=0; j<height; j++, ptr+=sizeof(JBLOCK)*width)
        jbrow[j] = (JBLOCKROW)ptr;
    JBLOCKARRAY jbarray = jbrow;
    pthread_t   pid[INDEX_THREAD_NUM];
    void        **para[INDEX_THREAD_NUM];
    uint64_t    ave_height  =   height/INDEX_THREAD_NUM;
    void        *size_part[INDEX_THREAD_NUM];

    for(int i=0; i<INDEX_THREAD_NUM; i++)
    {
        para[i] = (void**)malloc(sizeof(void*)*6);
        para[i][0] = (void*)width;
        para[i][1] = (void*)(uint64_t)(i*ave_height);
        para[i][2] = (void*)(uint64_t)((i == INDEX_THREAD_NUM-1)? (height-LBS):((i+1)*ave_height-1));
        para[i][3] = jbarray;
        para[i][4] = subBlockTab;
        para[i][5] = mutex;

        pthread_create(&pid[i], NULL, index_sub_thread, (void*)para[i]);
    }

    for(int i=0; i<INDEX_THREAD_NUM; i++)
    {
        pthread_join(pid[i], (void**)(&size_part[i]));
        free(para[i]);
        *size   +=  *((uint64_t*)size_part[i]);
        free(size_part[i]);
    }

    return subBlockTab;
}

static GHashTable **create_block_index(jpeg_coe_ptr base, uint64_t *size
    #ifdef FIX_OPTI
    , uint64_t fix_flag
    #endif
)
{
    GHashTable  **subBlockTab   =   (GHashTable**)malloc(sizeof(GHashTable*)*3);
    #ifdef FIX_OPTI
    if(fix_flag)
    {
    #endif
    pthread_mutex_t mutex[3];
    uint8_t     *ptr[3];
    uint64_t    width[3], height[3];
    for(int i=0; i<3; i++)
    {
        width[i] = base->imgSize[i*2];
        height[i] = base->imgSize[i*2+1];
    }
    ptr[0] = base->data;
    for(int i=1; i<3; i++)
        ptr[i] = ptr[i-1] + width[i-1]*height[i-1]*sizeof(JBLOCK);

    pthread_t   pid[3];
    void        **arg[3];
    uint64_t    size_part[3] = {0};
    for(int i=0; i<3; i++)
    {
        arg[i]  =   (void**)malloc(sizeof(void*)*5);
        pthread_mutex_init(&mutex[i], NULL);
        arg[i][0] = (void*)width[i];
        arg[i][1] = (void*)height[i];
        arg[i][2] = ptr[i];
        arg[i][3] = &mutex[i];
        arg[i][4] = &size_part[i];

        pthread_create(&pid[i], NULL, index_compute_thread, (void*)arg[i]);
    }

    *size   =   0;
    for(int i=0; i<3; i++)
    {
        pthread_join(pid[i], (void**)(&subBlockTab[i]));
        free(arg[i]);
        pthread_mutex_destroy(&mutex[i]);
        *size   +=  size_part[i];
    }
    #ifdef FIX_OPTI
    }
    else
    {
        for(int i=0; i<3; i++)
        {
            subBlockTab[i] = g_hash_table_new_full(g_int_hash,g_int_equal,free_item,free_digest_list);
        }
        *size   =   0;
    }
    #endif

    return subBlockTab;
}

void* index_thread(void *parameter)
{
    void        **arg       =   (void**)parameter;
    List        detectList  =   (List)arg[0];
    List        indexList   =   (List)arg[1];
    detectionDataPtr    detectPtr;

    #ifdef PART_TIME
    GTimer      *timer  =   g_timer_new();
    #endif

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
            #ifdef PART_TIME
            g_timer_start(timer);
            #endif

            uint64_t    tabSize;
            detectPtr->subBlockTab  =   
                create_block_index(((imagePtr)detectPtr->base->data)->decdData->targetInfo->coe, &tabSize
                #ifdef FIX_OPTI
                , detectPtr->fix_flag
                #endif
            );
            detectPtr->mem_size     +=  tabSize;

            #ifdef PART_TIME
            pthread_mutex_lock(&index_time_mutex);
            index_time +=  g_timer_elapsed(timer, NULL);
            pthread_mutex_unlock(&index_time_mutex);
            #endif

            detectPtr->next =   NULL;
            pthread_mutex_lock(&indexList->mutex);
            while(indexList->size < detectPtr->mem_size)
                pthread_cond_wait(&indexList->wCond, &indexList->mutex);
            if(indexList->counter)
                ((detectionDataPtr)indexList->tail)->next =   detectPtr;
            else
                indexList->head =   detectPtr;
            indexList->tail =   detectPtr;
            indexList->counter ++;
            indexList->size -=  detectPtr->mem_size;
            pthread_cond_signal(&indexList->rCond);
            pthread_mutex_unlock(&indexList->mutex);
        }
    }

    ESCAPE_LOOP:
    pthread_mutex_unlock(&indexList->mutex);

    #ifdef PART_TIME
    g_timer_destroy(timer);
    #endif

    pthread_mutex_lock(&indexList->mutex);
    indexList->ending ++;
    pthread_cond_signal(&indexList->rCond);
    pthread_mutex_unlock(&indexList->mutex);
}