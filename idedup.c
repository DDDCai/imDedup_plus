/*
 * @Author: Cai Deng
 * @Date: 2020-11-09 14:24:32
 * @LastEditors: Cai Deng dengcaidengcai@163.com
 * @LastEditTime: 2024-05-09 11:58:05
 * @Description: 为了实现使用原Huffman表压缩，在fse/huf_compress.c 中增加了HUF_compress_JPEGtab函数，并在huf.h 中增加了声明
 */
#include "idedup.h"
#include "2df.h"
#include "index.h"
#include "idelta.h"
#include "jpeg.h"
#include "rejpeg.h"
#include "buffer.h"

extern uint8_t chunking_mode;
extern int ROAD_NUM;
extern uint8_t in_chaos;
static uint32_t batch_id;
pthread_mutex_t batchID_mutex;

#define IDELTA 0
#define XDELTA 1
extern uint8_t delta_method;

#ifdef PART_TIME
extern double read_time;
extern double write_time;
extern pthread_mutex_t read_time_mutex, write_time_mutex;
#endif

#ifdef ORIGINAL_HUFF
typedef enum {
   HUF_repeat_none,  /**< Cannot use the previous table */
   HUF_repeat_check, /**< Can use the previous table but it must be checked. Note : The previous table must have been constructed by HUF_compress{1, 4}X_repeat */
   HUF_repeat_valid  /**< Can use the previous table and it is assumed to be valid */
 } HUF_repeat;

uint64_t HUF_compress_JPEGtab(void *src, uint32_t srcSize, void *dst, uint32_t dstSize, HUF_FSE *hufTab)
{
    unsigned workSpace[HUF_WORKSPACE_SIZE/sizeof(uint32_t)];
    HUF_repeat hp = HUF_repeat_valid;
    uint64_t   returnVal   =  HUF_compress4X_repeat(dst, dstSize, src, srcSize, 255, 11, workSpace, sizeof(workSpace), hufTab, &hp, 0, 0);
    if(returnVal==0 || 
       returnVal==1 ||  /* RLE. */
       HUF_isError(returnVal)
    )
        return 0;
    return returnVal;
}
#endif

uint32_t entropy_compress(void *src, uint32_t srcSize, void *dst, uint32_t dstSize)
{
    uint64_t   returnVal   =   
    #ifdef  HUFFMAN
        HUF_compress
    #endif
    #ifdef   FSE
        FSE_compress
    #endif
        (dst,dstSize,src,srcSize);

    if(returnVal==0 || 
       returnVal==1 ||  /* RLE. */
        #ifdef  HUFFMAN
            HUF_isError
        #endif
        #ifdef  FSE
            FSE_isError
        #endif
            (returnVal)
    )
        return 0;
    return returnVal;
}

/*---------------------------------------------------------------------*/

static void* dir_thread(void *parameter)
{
    void        **arg       =   (void**)parameter;
    char        *folderPath =   (char*)arg[0];
    List        dirList     =   (List)arg[1];
    DIR         *f_dir;
    struct      dirent      *f_entry,    *f_result = NULL;
    dirDataPtr  dirPtr;

    if(!(f_dir = opendir(folderPath)))
    {
        printf("fail to open folder %s\n", folderPath);
        exit(EXIT_FAILURE);
    }
    // while(!readdir_r(f_dir, &f_entry, &f_result) && f_result!=NULL)
    while(f_entry = readdir(f_dir))
    {
        if(!strcmp(f_entry->d_name,".") || !strcmp(f_entry->d_name,".."))
            continue;
        dirPtr  =   (dirDataPtr)malloc(sizeof(dirDataNode));
        dirPtr->next    =   NULL;
        strcpy(dirPtr->second_dir, f_entry->d_name);
        dirPtr->mem_size    =   sizeof(dirDataNode);

        pthread_mutex_lock(&dirList->mutex);
        while(dirList->size < dirPtr->mem_size)
            pthread_cond_wait(&dirList->wCond, &dirList->mutex);
        if(dirList->head)
            ((dirDataPtr)dirList->tail)->next   =   dirPtr;
        else 
            dirList->head   =   dirPtr;
        dirList->tail   =   dirPtr;
        dirList->counter    ++;
        dirList->size   -=  dirPtr->mem_size;
        pthread_cond_signal(&dirList->rCond);
        pthread_mutex_unlock(&dirList->mutex);
    }
    closedir(f_dir);

    pthread_mutex_lock(&dirList->mutex);
    dirList->ending =   1;
    pthread_cond_signal(&dirList->rCond);
    pthread_mutex_unlock(&dirList->mutex);
}

static void* name_thread(void *parameter)
{
    void        **arg       =   (void**)parameter;
    char        *folderPath =   (char*)arg[0];
    List        dirList     =   (List)arg[1];
    List        nameList    =   (List)arg[2];
    DIR         *s_dir;
    struct      dirent      s_entry,   *s_result = NULL;
    char        inPath[MAX_PATH_LEN];
    dirDataPtr  dirPtr;
    nameDataPtr namePtr;

    while(1)
    {
        pthread_mutex_lock(&dirList->mutex);
        while(dirList->counter == 0)
        {
            if(dirList->ending) goto ESCAPE_LOOP;
            pthread_cond_wait(&dirList->rCond, &dirList->mutex);
        }
        dirPtr  =   dirList->head;
        dirList->head   =   dirPtr->next;
        dirList->counter    --;
        dirList->size   +=  dirPtr->mem_size;
        pthread_cond_signal(&dirList->wCond);
        pthread_mutex_unlock(&dirList->mutex);

        if(dirPtr)
        {
            PUT_3_STRS_TOGETHER(inPath, folderPath, "/", dirPtr->second_dir);

            s_dir   =   opendir(inPath);
            uint8_t flag    =   1;
            if(!readdir_r(s_dir, &s_entry, &s_result) && s_result!=NULL)
            {
                while(flag)
                {
                    if(!strcmp(s_entry.d_name, ".") || !strcmp(s_entry.d_name, ".."))
                    {
                        if(readdir_r(s_dir, &s_entry, &s_result) || s_result==NULL)
                            flag    =   0;
                        continue;
                    }
                    namePtr =   (nameDataPtr)malloc(sizeof(nameDataNode));
                    namePtr->next   =   NULL;
                    strcpy(namePtr->second_dir, dirPtr->second_dir);
                    strcpy(namePtr->file_name, s_entry.d_name);
                    namePtr->mem_size   =   sizeof(nameDataNode);
                    if(!readdir_r(s_dir, &s_entry, &s_result) && s_result!=NULL)
                        namePtr->end_of_dir =   0;
                    else
                    {
                        namePtr->end_of_dir =   1;  /* default : no folder is empty! */
                        flag    =   0;
                    }

                    pthread_mutex_lock(&nameList->mutex);
                    while(nameList->size < namePtr->mem_size)
                        pthread_cond_wait(&nameList->wCond, &nameList->mutex);
                    if(nameList->head)
                        ((nameDataPtr)nameList->tail)->next =   namePtr;
                    else 
                        nameList->head  =   namePtr;
                    nameList->tail  =   namePtr;
                    nameList->counter   ++;
                    nameList->size  -=  namePtr->mem_size;
                    pthread_cond_signal(&nameList->rCond);
                    pthread_mutex_unlock(&nameList->mutex);
                }
            }

            closedir(s_dir);
            free(dirPtr);
        }
    }

    ESCAPE_LOOP:
    pthread_mutex_unlock(&dirList->mutex);

    pthread_mutex_lock(&nameList->mutex);
    nameList->ending    ++;
    pthread_cond_signal(&nameList->rCond);
    pthread_mutex_unlock(&nameList->mutex);
}

static void* read_thread(void *parameter)
{
    void        **arg       =   (void**)parameter;
    List        nameList    =   (List)arg[0];
    List        rawList     =   (List)arg[1];
    char        *folderPath =   (char*)arg[2];
    nameDataPtr namePtr;
    rawDataPtr  rawPtr;
    char        filePath[MAX_PATH_LEN];
    uint8_t     *rawDataBuffer;
    uint64_t    fileSize;
    struct      stat        statbuf;
    FILE        *fp;
    uint64_t    *rawSize    =   (uint64_t*)g_malloc0(sizeof(uint64_t));
    uint8_t     end_of_dir;
    #ifdef PART_TIME
    GTimer      *timer      =   g_timer_new();
    #endif

    while(1)
    {
        pthread_mutex_lock(&nameList->mutex);
        while(nameList->counter == 0)
        {
            if(nameList->ending) goto ESCAPE_LOOP;
            pthread_cond_wait(&nameList->rCond, &nameList->mutex);
        }
        namePtr =   nameList->head;
        nameList->head  =   namePtr->next;
        nameList->counter   --;
        nameList->size  +=  namePtr->mem_size;
        pthread_cond_signal(&nameList->wCond);
        pthread_mutex_unlock(&nameList->mutex);

        if(namePtr)
        {
            #ifdef PART_TIME
            g_timer_start(timer);
            #endif

            end_of_dir  =   namePtr->end_of_dir;
            rawPtr      =   (rawDataPtr)malloc(sizeof(rawDataNode));

            PUT_3_STRS_TOGETHER(filePath, folderPath, "/", namePtr->second_dir);
            rawPtr->dir_name    =   (char*)malloc(strlen(filePath) + 1);
            strcpy(rawPtr->dir_name, filePath);
            PUT_3_STRS_TOGETHER(filePath, filePath, "/", namePtr->file_name);
            stat(filePath, &statbuf);
            fileSize    =   statbuf.st_size;
            rawDataBuffer   =   (uint8_t*)malloc(fileSize);
            fp  =   fopen(filePath, "rb");
            if(1 != fread(rawDataBuffer, fileSize, 1, fp))
            {
                printf("fail to read %s\n", filePath);
                free(rawDataBuffer);
                free(rawPtr);
                fclose(fp);
                continue ;
            }
            fclose(fp);

            rawPtr->data    =   rawDataBuffer;
            rawPtr->size    =   fileSize;
            rawPtr->name    =   (char*)malloc(strlen(namePtr->file_name) + 1 + strlen(namePtr->second_dir));
            // for web-b only.
            // strcpy(rawPtr->name, namePtr->second_dir);
            // strcat(rawPtr->name, namePtr->file_name);
            rawPtr->name    =   (char*)malloc(strlen(namePtr->file_name) + 1);
            strcpy(rawPtr->name, namePtr->file_name);
            rawPtr->mem_size=   sizeof(rawDataNode) + fileSize + strlen(namePtr->file_name) + strlen(rawPtr->dir_name) + 2;
            rawPtr->next    =   NULL;

            #ifdef PART_TIME
            pthread_mutex_lock(&read_time_mutex);
            read_time   +=  g_timer_elapsed(timer, NULL);
            pthread_mutex_unlock(&read_time_mutex);
            #endif

            pthread_mutex_lock(&rawList->mutex);
            while(rawList->size < rawPtr->mem_size)
                pthread_cond_wait(&rawList->wCond, &rawList->mutex);
            if(rawList->head)
                ((rawDataPtr)rawList->tail)->next   =   rawPtr;
            else
                rawList->head   =   rawPtr;
            rawList->tail   =   rawPtr;
            rawList->counter    ++;
            rawList->size   -=  rawPtr->mem_size;
            pthread_cond_signal(&rawList->rCond);
            pthread_mutex_unlock(&rawList->mutex);
            // free(rawPtr->data);
            // free(rawPtr->name);
            // free(rawPtr);

            *rawSize +=  fileSize;
            free(namePtr);
        }

        if(chunking_mode)
        {   /* necessary brackets. */
            if(*rawSize >= PATCH_SIZE/READ_THREAD_NUM) goto ESCAPE_LOOP_1;
        }
        else
            if(end_of_dir && (*rawSize >= PATCH_SIZE))  goto ESCAPE_LOOP_1;
    }

    ESCAPE_LOOP:
    pthread_mutex_unlock(&nameList->mutex);
    ESCAPE_LOOP_1:

    #ifdef PART_TIME
    g_timer_destroy(timer);
    #endif

    pthread_mutex_lock(&rawList->mutex);
    rawList->ending  ++;
    for(int i=0; i<DECODE_THREAD_NUM; i++)
        pthread_cond_signal(&rawList->rCond);
    pthread_mutex_unlock(&rawList->mutex);

    return  (void*)rawSize;
}

typedef struct {
    void *buffer;
    uint8_t *ptr;
    uint64_t leftSize, size;

}   idedup_cache;

void write_cache(idedup_cache *cache, uint8_t *data, uint64_t size, FILE *fp)
{
    if(size > cache->size)
    {
        fwrite(data, size, 1, fp);
        fflush(fp);
        fsync(fileno(fp));
    }
    else
    {
        if(size > cache->leftSize)
        {
            fwrite(cache->buffer, cache->size-cache->leftSize, 1, fp);
            fflush(fp);
            fsync(fileno(fp));

            cache->leftSize =   cache->size;
            cache->ptr      =   cache->buffer;
        }
        memcpy(cache->ptr, data, size);
        cache->ptr  +=  size;
        cache->leftSize -=  size;
    }
}

static void* write_thread(void *parameter)
{
    void        **arg       =   (void**)parameter;
    List        rejpegList  =   (List)arg[0];
    char        *outPath    =   (char*)arg[1];
    uint64_t    *finalSize  =   (uint64_t*)g_malloc0(sizeof(uint64_t));
    char        outFilePath[MAX_PATH_LEN];
    dedupResPtr dedupPtr;
    PUT_3_STRS_TOGETHER(outFilePath, outPath, ".", "idp");
    FILE        *fp         =   fopen(outFilePath, "ab");;
    if(fp == NULL) printf("%s\n",outFilePath);
    rejpegResPtr    rejpegPtr;

    idedup_cache cache;
    cache.size  =   PATCH_SIZE>>2;
    cache.leftSize  =   cache.size;
    cache.buffer    =   malloc(cache.size);
    cache.ptr   =   (uint8_t*)cache.buffer;

    #ifdef DEBUG_1
    free(finalSize);
    finalSize   =   (uint64_t*)g_malloc0(sizeof(uint64_t)*9);
    #endif

    #ifdef PART_TIME
    GTimer      *timer  =   g_timer_new();
    #endif

    while(1)
    {
        pthread_mutex_lock(&rejpegList->mutex);
        while(rejpegList->counter == 0)
        {
            if(rejpegList->ending == REJPEG_THREAD_NUM) goto ESCAPE_LOOP;
            pthread_cond_wait(&rejpegList->rCond, &rejpegList->mutex);
        }
        rejpegPtr   =   rejpegList->head;
        rejpegList->head    =   rejpegPtr->next;
        rejpegList->counter --;
        rejpegList->size    +=  rejpegPtr->mem_size;
        for(int i=0; i<REJPEG_THREAD_NUM; i++)
            pthread_cond_signal(&rejpegList->wCond);
        pthread_mutex_unlock(&rejpegList->mutex);

        if(rejpegPtr)
        {
            #ifdef PART_TIME
            g_timer_start(timer);
            #endif

            dedupPtr    =   rejpegPtr->dedupRes;
            uint32_t    subSize[]   =   {
                dedupPtr->imgSize[0], 
                dedupPtr->imgSize[1], 
                dedupPtr->imgSize[2], 
                dedupPtr->imgSize[3],
                dedupPtr->y_counter, 
                dedupPtr->u_counter, 
                dedupPtr->v_counter,
                dedupPtr->headerSize, 
                #ifdef COMPRESS_DELTA_INS
                rejpegPtr->cpxSize,
                rejpegPtr->cpySize,
                rejpegPtr->cplSize,
                rejpegPtr->inlSize,
                #else
                dedupPtr->copy_x->len*sizeof(COPY_X), 
                dedupPtr->copy_y->len*sizeof(COPY_Y),
                dedupPtr->copy_l->len*sizeof(COPY_L), 
                dedupPtr->insert_l->len*sizeof(INSERT_L), 
                #endif
                rejpegPtr->rejpegSize
            };

            // #ifndef DO_NOT_WRITE
            // PUT_3_STRS_TOGETHER(outFilePath, outPath, "/", dedupPtr->name);
            // strcat(outFilePath, ".sid");
            // fp  =   fopen(outFilePath, "wb");
            // fwrite(dedupPtr->baseName,      1, strlen(dedupPtr->baseName)+1, fp);
            // fwrite(subSize,                 1, sizeof(subSize), fp);
            // fwrite(&dedupPtr->ffxx,         1, 1,               fp);
            // fwrite(&dedupPtr->xx,           1, 1,               fp);
            // fwrite(dedupPtr->header,        1, subSize[7],      fp);
            // #ifdef COMPRESS_DELTA_INS
            // fwrite(&rejpegPtr->flag,        1, 1,               fp);
            // if(rejpegPtr->cpx)
            // {
            //     fwrite(rejpegPtr->cpx,      1, subSize[8],      fp);
            //     free(rejpegPtr->cpx);
            // }
            // else 
            //     fwrite(dedupPtr->copy_x->data,  1,  subSize[8], fp);
            // if(rejpegPtr->cpy)
            // {
            //     fwrite(rejpegPtr->cpy,      1, subSize[9],      fp);
            //     free(rejpegPtr->cpy);
            // }
            // else 
            //     fwrite(dedupPtr->copy_y->data,  1,  subSize[9], fp);
            // if(rejpegPtr->cpl)
            // {
            //     fwrite(rejpegPtr->cpl,      1, subSize[10],     fp);
            //     free(rejpegPtr->cpl);
            // }
            // else 
            //     fwrite(dedupPtr->copy_l->data,  1,  subSize[10],fp);
            // if(rejpegPtr->inl)
            // {
            //     fwrite(rejpegPtr->inl,      1, subSize[11],     fp);
            //     free(rejpegPtr->inl);
            // }
            // else 
            //     fwrite(dedupPtr->insert_l->data, 1, subSize[11],fp);
            // #else
            // fwrite(dedupPtr->copy_x->data,  1, subSize[8],      fp);
            // fwrite(dedupPtr->copy_y->data,  1, subSize[9],      fp);
            // fwrite(dedupPtr->copy_l->data,  1, subSize[10],     fp);
            // fwrite(dedupPtr->insert_l->data,1, subSize[11],     fp);
            // #endif
            // fwrite(rejpegPtr->rejpegRes,    1, subSize[12],     fp);
            // fflush(fp);
            // fsync(fileno(fp));
            // fclose(fp);
            // #endif


            #ifndef DO_NOT_WRITE
            write_cache(&cache, dedupPtr->baseName, strlen(dedupPtr->baseName)+1, fp);
            write_cache(&cache, dedupPtr->name, strlen(dedupPtr->name)+1, fp);
            write_cache(&cache, (uint8_t*)subSize,            sizeof(subSize), fp);
            write_cache(&cache, &dedupPtr->ffxx,    1             , fp);
            write_cache(&cache, &dedupPtr->xx,      1             , fp);
            write_cache(&cache, dedupPtr->header,   subSize[7]    , fp);
            #ifdef COMPRESS_DELTA_INS
            write_cache(&cache, &rejpegPtr->flag,        1, fp);
            if(rejpegPtr->cpx)
            {
                write_cache(&cache, rejpegPtr->cpx,      subSize[8], fp);
                free(rejpegPtr->cpx);
            }
            else 
                write_cache(&cache, dedupPtr->copy_x->data,  subSize[8], fp);
            if(rejpegPtr->cpy)
            {
                write_cache(&cache, rejpegPtr->cpy,   subSize[9], fp);
                free(rejpegPtr->cpy);
            }
            else 
                write_cache(&cache, dedupPtr->copy_y->data,    subSize[9], fp);
            if(rejpegPtr->cpl)
            {
                write_cache(&cache, rejpegPtr->cpl,       subSize[10], fp);
                free(rejpegPtr->cpl);
            }
            else 
                write_cache(&cache, dedupPtr->copy_l->data,    subSize[10], fp);
            if(rejpegPtr->inl)
            {
                write_cache(&cache, rejpegPtr->inl,       subSize[11], fp);
                free(rejpegPtr->inl);
            }
            else 
                write_cache(&cache, dedupPtr->insert_l->data,  subSize[11], fp);
            #else
            write_cache(&cache, dedupPtr->copy_x->data,   subSize[8], fp);
            write_cache(&cache, dedupPtr->copy_y->data,   subSize[9], fp);
            write_cache(&cache, dedupPtr->copy_l->data,   subSize[10], fp);
            write_cache(&cache, dedupPtr->insert_l->data, subSize[11], fp);
            #endif
            write_cache(&cache, rejpegPtr->rejpegRes,     subSize[12], fp);
            #endif


            *finalSize += (
                strlen(dedupPtr->baseName)+1+
                //
                strlen(dedupPtr->name)+1+
                sizeof(subSize)+
                2+
                subSize[7]+
                #ifdef COMPRESS_DELTA_INS
                1+
                #endif
                subSize[8]+
                subSize[9]+
                subSize[10]+
                subSize[11]+
                subSize[12]
            );

            #ifdef DEBUG_1
            finalSize[1] +=  (strlen(dedupPtr->baseName)
                                // +3
                                +strlen(dedupPtr->name)
                                +4
                                +sizeof(subSize))
                                #ifdef COMPRESS_DELTA_INS
                                +   1
                                #endif
                                ;
            finalSize[2] +=  subSize[7];
            finalSize[3] +=  subSize[8];
            finalSize[4] +=  subSize[9];
            finalSize[5] +=  subSize[10];
            finalSize[6] +=  subSize[11];
            finalSize[7] +=  subSize[12];
            finalSize[8] =   finalSize[0];
            #endif

            g_array_free(dedupPtr->copy_x, TRUE);
            g_array_free(dedupPtr->copy_y, TRUE);
            g_array_free(dedupPtr->copy_l, TRUE);
            g_array_free(dedupPtr->insert_l, TRUE);
            g_array_free(dedupPtr->insert_p, TRUE);
            #ifdef HEADER_DELTA
            free(dedupPtr->header);
            #endif
            free(dedupPtr);
            free(rejpegPtr->rejpegRes);
            free(rejpegPtr);

            #ifdef PART_TIME
            pthread_mutex_lock(&write_time_mutex);
            write_time  +=  g_timer_elapsed(timer, NULL);
            pthread_mutex_unlock(&write_time_mutex);
            #endif
        }
    }

    ESCAPE_LOOP:
    pthread_mutex_unlock(&rejpegList->mutex);

    #ifdef PART_TIME
    g_timer_start(timer);
    #endif
    #ifndef DO_NOT_WRITE
    fwrite(cache.buffer, cache.size-cache.leftSize, 1, fp);
    fflush(fp);
    fsync(fileno(fp));
    #endif
    fclose(fp);
    free(cache.buffer);
    #ifdef PART_TIME
    pthread_mutex_lock(&write_time_mutex);
    write_time  +=  g_timer_elapsed(timer, NULL);
    pthread_mutex_unlock(&write_time_mutex);
    #endif

    #ifdef PART_TIME
    g_timer_destroy(timer);
    #endif

    return  (void*)finalSize;
}

static void* tra_write_thread(void *parameter)
{
    void        **arg       =   (void**)parameter;
    List        rejpegList  =   (List)arg[0];
    char        *outPath    =   (char*)arg[1];
    uint64_t    *finalSize  =   (uint64_t*)g_malloc0(sizeof(uint64_t));
    char        outFilePath[MAX_PATH_LEN];
    dedupResPtr dedupPtr;
    PUT_3_STRS_TOGETHER(outFilePath, outPath, ".", "idp");
    FILE        *fp         =   fopen(outFilePath, "ab");;
    rejpegResPtr    rejpegPtr;

    idedup_cache cache;
    cache.size  =   PATCH_SIZE>>2;
    cache.leftSize  =   cache.size;
    cache.buffer    =   (uint8_t*)malloc(cache.size);
    cache.ptr   =   cache.buffer;

    #ifdef DEBUG_1
    free(finalSize);
    finalSize   =   (uint64_t*)g_malloc0(sizeof(uint64_t)*9);
    #endif

    #ifdef PART_TIME
    GTimer      *timer  =   g_timer_new();
    #endif

    while(1)
    {
        pthread_mutex_lock(&rejpegList->mutex);
        while(rejpegList->counter == 0)
        {
            if(rejpegList->ending == REJPEG_THREAD_NUM) goto ESCAPE_LOOP;
            pthread_cond_wait(&rejpegList->rCond, &rejpegList->mutex);
        }
        rejpegPtr   =   rejpegList->head;
        rejpegList->head    =   rejpegPtr->next;
        rejpegList->counter --;
        rejpegList->size    +=  rejpegPtr->mem_size;
        for(int i=0; i<REJPEG_THREAD_NUM; i++)
            pthread_cond_signal(&rejpegList->wCond);
        pthread_mutex_unlock(&rejpegList->mutex);

        if(rejpegPtr)
        {
            #ifdef PART_TIME
            g_timer_start(timer);
            #endif

            dedupPtr    =   rejpegPtr->dedupRes;

            uint32_t    subSize[]   =   {
                dedupPtr->imgSize[0], 
                dedupPtr->imgSize[1], 
                dedupPtr->imgSize[2], 
                dedupPtr->imgSize[3],
                dedupPtr->headerSize, 
                dedupPtr->v_counter
            };

            // #ifndef DO_NOT_WRITE
            // PUT_3_STRS_TOGETHER(outFilePath, outPath, "/", dedupPtr->name);
            // strcat(outFilePath, ".sid");
            // fp  =   fopen(outFilePath, "wb");
            // fwrite(dedupPtr->baseName,      1, strlen(dedupPtr->baseName)+1, fp);
            // fwrite(subSize,                 1, sizeof(subSize), fp);
            // fwrite(&dedupPtr->ffxx,         1, 1,               fp);
            // fwrite(&dedupPtr->xx,           1, 1,               fp);
            // fwrite(dedupPtr->header,        1, subSize[4],      fp);
            // fwrite(dedupPtr->insert_l,      1, subSize[5],      fp);
            // fflush(fp);
            // fsync(fileno(fp));
            // fclose(fp);
            // #endif

            #ifndef DO_NOT_WRITE
            write_cache(&cache, dedupPtr->baseName,       strlen(dedupPtr->baseName)+1, fp);
            write_cache(&cache, (uint8_t*)subSize,                  sizeof(subSize), fp);
            write_cache(&cache, &dedupPtr->ffxx,          1             , fp);
            write_cache(&cache, &dedupPtr->xx,            1             , fp);
            write_cache(&cache, dedupPtr->header,         subSize[4]    , fp);
            write_cache(&cache, (uint8_t*)dedupPtr->insert_l,       subSize[5]    , fp);
            #endif

            *finalSize += (
                strlen(dedupPtr->baseName)+1+
                sizeof(subSize)+
                2+
                subSize[4]+
                subSize[5]
            );

            #ifdef DEBUG_1
            finalSize[1] +=  (strlen(dedupPtr->baseName)+3+sizeof(subSize))
                                #ifdef COMPRESS_DELTA_INS
                                +   1
                                #endif
                                ;
            finalSize[2] +=  subSize[4];
            finalSize[3] +=  subSize[5];
            #endif

            g_array_free(dedupPtr->insert_p, TRUE);
            #ifdef HEADER_DELTA
            free(dedupPtr->header);
            #endif
            free(dedupPtr->insert_l);
            free(dedupPtr);
            free(rejpegPtr);

            #ifdef PART_TIME
            pthread_mutex_lock(&write_time_mutex);
            write_time  +=  g_timer_elapsed(timer, NULL);
            pthread_mutex_unlock(&write_time_mutex);
            #endif
        }
    }

    ESCAPE_LOOP:
    pthread_mutex_unlock(&rejpegList->mutex);

    #ifdef PART_TIME
    g_timer_start(timer);
    #endif
    fwrite(cache.buffer, cache.size-cache.leftSize, 1, fp);
    fflush(fp);
    fsync(fileno(fp));
    fclose(fp);
    free(cache.buffer);
    #ifdef PART_TIME
    pthread_mutex_lock(&write_time_mutex);
    write_time  +=  g_timer_elapsed(timer, NULL);
    pthread_mutex_unlock(&write_time_mutex);
    #endif

    #ifdef PART_TIME
    g_timer_destroy(timer);
    #endif

    return  (void*)finalSize;
}

static void free_ht_val(gpointer p)
{
    g_ptr_array_free(p, TRUE);
}

void* compress_road_thread(void *parameter)
{
    void        **arg       =   (void**)parameter;
    char        *inFolder   =   (char*)arg[0];
    char        *outFolder  =   (char*)arg[1];
    List        dirList;
    List        nameList;
    pthread_t   name_t_id;
    if(in_chaos)
        nameList    =   (List)arg[2];
    else
    {
        dirList =   (List)arg[2];
        INIT_LIST(nameList, NAME_LIST_MAX);
        void        *name_arg[] =   {inFolder, dirList, nameList};
        pthread_create(&name_t_id, NULL, name_thread, (void*)name_arg);
    }
    uint64_t    *result     =   (uint64_t*)arg[3];
    char        outPath[MAX_PATH_LEN];

    while(1)
    {
        pthread_mutex_lock(&nameList->mutex);
        while(nameList->counter == 0)
        {
            if(nameList->ending) goto ESCAPE_LOOP;
            pthread_cond_wait(&nameList->rCond, &nameList->mutex);
        }
        pthread_mutex_unlock(&nameList->mutex);

        pthread_mutex_lock(&batchID_mutex);
        sprintf(outPath, "%s/%u", outFolder, batch_id);
        batch_id ++;
        pthread_mutex_unlock(&batchID_mutex);
        if(access(outPath, 0) < 0)
            mkdir(outPath, 0755);
        
        Buffer      decodeBuffer;
        decodeBuffer.head   =   NULL;
        decodeBuffer.tail   =   NULL;
        decodeBuffer.size   =   DECODE_BUFFER_SIZE;
        pthread_mutex_init(&decodeBuffer.mutex, NULL);

        // List        rawList, decList, detList[MIDDLE_THREAD_NUM], indList[MIDDLE_THREAD_NUM],
        //             dupList[MIDDLE_THREAD_NUM], rejList;
        List        rawList, decList, detList[MIDDLE_THREAD_NUM], indList[MIDDLE_THREAD_NUM],
                    dupList, rejList;
        INIT_LIST(rawList, READ_LIST_MAX);
        INIT_LIST(decList, DECD_LIST_MAX);
        for(int i=0; i<MIDDLE_THREAD_NUM; i++)
        {
            INIT_LIST(detList[i], DECT_LIST_MAX/MIDDLE_THREAD_NUM);
            INIT_LIST(indList[i], INDX_LIST_MAX/MIDDLE_THREAD_NUM);
        }
        INIT_LIST(dupList, DEUP_LIST_MAX);
        INIT_LIST(rejList, REJG_LIST_MAX);

        GHashTable      *featureT[SF_NUM];
        pthread_mutex_t ftMutex[SF_NUM];
        for(int i=0; i<SF_NUM; i++)
        {
            featureT[i] =   g_hash_table_new_full(g_int64_hash, g_int64_equal, NULL, free_ht_val);
            pthread_mutex_init(&ftMutex[i], NULL);
        }

        pthread_t   read_t_id[READ_THREAD_NUM], decd_t_id[DECODE_THREAD_NUM], detc_t_id[MIDDLE_THREAD_NUM], indx_t_id[MIDDLE_THREAD_NUM],
                    dedup_t_id[MIDDLE_THREAD_NUM], rejpg_t_id[REJPEG_THREAD_NUM], writ_t_id[WRITE_THREAD_NUM];

        void        *read_arg[] = {nameList, rawList, inFolder};
        void        **decd_arg[DECODE_THREAD_NUM], **detc_arg[MIDDLE_THREAD_NUM], **indx_arg[MIDDLE_THREAD_NUM], **dedu_arg[MIDDLE_THREAD_NUM], **reje_arg[REJPEG_THREAD_NUM];
        for(int i=0; i<DECODE_THREAD_NUM; i++)
        {
            decd_arg[i] = (void**)malloc(sizeof(void*)*3);
            decd_arg[i][0] = outPath;
            decd_arg[i][1] = rawList;
            decd_arg[i][2] = decList;
        }
        for(int i=0; i<MIDDLE_THREAD_NUM; i++)
        {
            detc_arg[i] = (void**)malloc(sizeof(void*)*6);
            indx_arg[i] = (void**)malloc(sizeof(void*)*2);
            dedu_arg[i] = (void**)malloc(sizeof(void*)*3);
            detc_arg[i][0] = decList;
            detc_arg[i][1] = detList[i];
            detc_arg[i][2] = outPath;
            detc_arg[i][3] = featureT;
            detc_arg[i][4] = ftMutex;
            detc_arg[i][5] = &decodeBuffer;
            indx_arg[i][0] = detList[i];
            indx_arg[i][1] = indList[i];
            if(delta_method == IDELTA)
                dedu_arg[i][0] = indList[i];
            else
                dedu_arg[i][0] = detList[i];
            dedu_arg[i][1] = dupList;
            dedu_arg[i][2] = outPath;
        }
        for(int i=0; i<REJPEG_THREAD_NUM; i++)
        {
            reje_arg[i] = (void**)malloc(sizeof(void*)*3);
            reje_arg[i][0] = dupList;
            reje_arg[i][1] = rejList;
            reje_arg[i][2] = outPath;
        }
        void        *writ_arg[] = {rejList, outPath};

        void        *rawSize[READ_THREAD_NUM], *undecdSize[DECODE_THREAD_NUM], *deltaedSize[MIDDLE_THREAD_NUM],
                    *finalSize[WRITE_THREAD_NUM], *unhandledSize[MIDDLE_THREAD_NUM];

        for(int i=0; i<READ_THREAD_NUM; i++)
            pthread_create(&read_t_id[i], NULL, read_thread, (void*)read_arg);
        for(int i=0; i<DECODE_THREAD_NUM; i++)
            pthread_create(&decd_t_id[i], NULL, decode_thread, (void*)decd_arg[i]);
        // if(delta_method == XDELTA)
        // {
        //     for(int i=0; i<MIDDLE_THREAD_NUM; i++)
        //         pthread_create(&detc_t_id[i], NULL, tra_detect_thread, (void*)detc_arg[i]);
        // }
        // else
        // {
            for(int i=0; i<MIDDLE_THREAD_NUM; i++)
                pthread_create(&detc_t_id[i], NULL, detect_thread, (void*)detc_arg[i]);
        // }
        if(delta_method == IDELTA)
        {
            for(int i=0; i<MIDDLE_THREAD_NUM; i++)
                pthread_create(&indx_t_id[i], NULL, index_thread, (void*)indx_arg[i]);
        }
        if(delta_method == XDELTA)
        {
            for(int i=0; i<MIDDLE_THREAD_NUM; i++)
                pthread_create(&dedup_t_id[i], NULL, tra_dedup_thread, (void*)dedu_arg[i]);
        }
        else 
        {
            for(int i=0; i<MIDDLE_THREAD_NUM; i++)
                pthread_create(&dedup_t_id[i], NULL, dedup_thread, (void*)dedu_arg[i]);
        }
        if(delta_method == XDELTA)
        {
            for(int i=0; i<REJPEG_THREAD_NUM; i++)
                pthread_create(&rejpg_t_id[i], NULL, tra_rejpeg_thread, (void*)reje_arg[i]);
        }
        else 
        {
            for(int i=0; i<REJPEG_THREAD_NUM; i++)
                pthread_create(&rejpg_t_id[i], NULL, rejpeg_thread, (void*)reje_arg[i]);
        }
        if(delta_method == XDELTA)
        {
            for(int i=0; i<WRITE_THREAD_NUM; i++)
                pthread_create(&writ_t_id[i], NULL ,tra_write_thread, (void*)writ_arg);
        }
        else 
        {
            for(int i=0; i<WRITE_THREAD_NUM; i++)
                pthread_create(&writ_t_id[i], NULL ,write_thread, (void*)writ_arg);
        }

        for(int i=0; i<READ_THREAD_NUM; i++)
            pthread_join(read_t_id[i], (void**)(&rawSize[i]));
        for(int i=0; i<DECODE_THREAD_NUM; i++)
            pthread_join(decd_t_id[i], (void**)(&undecdSize[i]));
        for(int i=0; i<MIDDLE_THREAD_NUM; i++)
            pthread_join(detc_t_id[i], (void**)(&unhandledSize[i]));
        if(delta_method == IDELTA)
        {
            for(int i=0; i<MIDDLE_THREAD_NUM; i++)
                pthread_join(indx_t_id[i], NULL);
        }
        for(int i=0; i<MIDDLE_THREAD_NUM; i++)
            pthread_join(dedup_t_id[i], (void**)(&deltaedSize[i]));
        for(int i=0; i<REJPEG_THREAD_NUM; i++)
            pthread_join(rejpg_t_id[i], NULL);
        for(int i=0; i<WRITE_THREAD_NUM; i++)
            pthread_join(writ_t_id[i], (void**)(&finalSize[i]));

        for(int i=0; i<SF_NUM; i++)
        {
            g_hash_table_destroy(featureT[i]);
            pthread_mutex_destroy(&ftMutex[i]);
        }

        pthread_mutex_destroy(&decodeBuffer.mutex);

        for(int i=0; i<READ_THREAD_NUM; i++)
            result[0] += *((uint64_t*)rawSize[i]);
        for(int i=0; i<DECODE_THREAD_NUM; i++)
        {
            result[1] += *((uint64_t*)undecdSize[i]);
            result[12] += ((uint64_t*)undecdSize[i])[1];
        }
        for(int i=0; i<MIDDLE_THREAD_NUM; i++)
        {
            result[13] += ((uint64_t*)deltaedSize[i])[0];
            result[14] += ((uint64_t*)deltaedSize[i])[1];
            result[16] += ((uint64_t*)deltaedSize[i])[2];
            result[2]  += ((uint64_t*)deltaedSize[i])[2];
        }
        for(int i=0; i<MIDDLE_THREAD_NUM; i++)
        {
            result[2] += *((uint64_t*)unhandledSize[i]);
            result[11] += *((uint64_t*)unhandledSize[i]);
            result[15] += ((uint64_t*)unhandledSize[i])[1];
        }
        for(int i=0; i<WRITE_THREAD_NUM; i++)
            result[2] += *((uint64_t*)finalSize[i]);
        #ifdef DEBUG_1
        for(int j=0; j<WRITE_THREAD_NUM; j++)
            for(int i=0; i<8; i++)
                result[3+i] += ((uint64_t*)finalSize[j])[1+i];
        #endif

        for(int i=0; i<READ_THREAD_NUM; i++)
            free(rawSize[i]);
        for(int i=0; i<DECODE_THREAD_NUM; i++)
            free(undecdSize[i]);
        for(int i=0; i<MIDDLE_THREAD_NUM; i++)
        {
            free(unhandledSize[i]);
            free(deltaedSize[i]);
        }
        for(int i=0; i<WRITE_THREAD_NUM; i++)
            free(finalSize[i]);
        
        DESTROY_LIST(rawList);
        DESTROY_LIST(decList);
        for(int i=0; i<DECODE_THREAD_NUM; i++)
        {
            free(decd_arg[i]);
        }
        for(int i=0; i<MIDDLE_THREAD_NUM; i++)
        {
            DESTROY_LIST(detList[i]);
            DESTROY_LIST(indList[i]);
            free(detc_arg[i]);
            free(indx_arg[i]);
            free(dedu_arg[i]);
            free(reje_arg[i]);
        }
        DESTROY_LIST(dupList);
        DESTROY_LIST(rejList);
    }

    ESCAPE_LOOP:
    pthread_mutex_unlock(&nameList->mutex);

    if(!in_chaos)
    {
        pthread_join(name_t_id, NULL);
        DESTROY_LIST(nameList);
    }
}

uint64_t* idedup_compress(char *inFolder, char *outFolder)
{
    // struct      rabin_t     *h  =   rabin_init(sizeof(JBLOCK));
    // rabin_free(h);
    List        dirList;
    INIT_LIST(dirList, NAME_LIST_MAX);
    pthread_t   dir_t_id;
    void        *dir_arg[]  =   {inFolder, dirList};
    pthread_create(&dir_t_id, NULL, dir_thread, (void*)dir_arg);

    List        nameList;
    pthread_t   name_t_id;
    if(in_chaos)
    {
        INIT_LIST(nameList, NAME_LIST_MAX);
        void        *name_arg[] =   {inFolder, dirList, nameList};
        pthread_create(&name_t_id, NULL, name_thread, (void*)name_arg);
    }

    #ifdef DEBUG_1
    uint64_t    *result =   (uint64_t*)g_malloc0(sizeof(uint64_t)*17);
    #else
    uint64_t    *result =   (uint64_t*)g_malloc0(sizeof(uint64_t)*3);
    #endif
    batch_id = 0;
    pthread_mutex_init(&batchID_mutex, NULL);

    pthread_t   road_t_id[ROAD_NUM];
    void        **road_arg[ROAD_NUM];
    for(int i=0; i<ROAD_NUM; i++)
    {
        road_arg[i] =   (void**)malloc(sizeof(void*)*4);
        road_arg[i][0]  =   inFolder;
        road_arg[i][1]  =   outFolder;
        if(in_chaos)
            road_arg[i][2]  =   nameList;
        else
            road_arg[i][2]  =   dirList;
        #ifdef DEBUG_1
        road_arg[i][3]  =   g_malloc0(sizeof(uint64_t)*17);
        #else
        road_arg[i][3]  =   g_malloc0(sizeof(uint64_t)*3);
        #endif
        pthread_create(&road_t_id[i], NULL, compress_road_thread, (void*)road_arg[i]);
    }
    for(int i=0; i<ROAD_NUM; i++)
        pthread_join(road_t_id[i], NULL);

    pthread_join(dir_t_id, NULL);
    DESTROY_LIST(dirList);
    if(in_chaos)
    {
        pthread_join(name_t_id, NULL);
        DESTROY_LIST(nameList);
    }

    pthread_mutex_destroy(&batchID_mutex);

    for(int i=0; i<ROAD_NUM; i++)
    {
        #ifdef DEBUG_1
        for(int j=0; j<17; j++)
        #else 
        for(int j=0; j<3; j++)
        #endif
        {
            result[j]   +=  ((uint64_t*)road_arg[i][3])[j];
        }
        free(road_arg[i][3]);
        free(road_arg[i]);
    }
    
    printf("average chunk size : %f GB\n", result[0]/1024.0/1024.0/1024.0/batch_id);
    printf("batch counter : %d\n", batch_id);

    return result;
}

/*---------------------------------------------------------------------*/
/*---------------------------------------------------------------------*/
/*---------------------------------------------------------------------*/
/*---- Above is about compression and decompression is as follows. ----*/
/*---------------------------------------------------------------------*/
/*---------------------------------------------------------------------*/
/*---------------------------------------------------------------------*/

#define CHECK_POSTFIX(fileName,lastName)  (!strcmp(lastName,(fileName) + strlen((fileName)) - strlen((lastName))))

// void* de_read_thread(void *parameter)
// {
//     void        **arg   =   (void**)parameter;
//     char        *inPath =   (char*)arg[0];
//     List        readList=   (List)arg[1];
//     List        tabList =   (List)arg[2];
//     GHashTable  *coeTab =   (GHashTable*)tabList->head;
//     pthread_mutex_t mutex  =   tabList->mutex;
//     struct      stat    stbuf;
//     DIR         *dir;
//     struct      dirent  *entry;
//     FILE        *fp;
//     char        *name;
//     char        filePath[MAX_PATH_LEN];
//     uint8_t     *buf, *ptr;
//     de_readPtr  readData;
//     jpeg_coe_ptr    coe;
//     uint64_t    *returnSize =   (uint64_t*)g_malloc0(sizeof(uint64_t));

//     if(!(dir = opendir(inPath)))
//     {
//         printf("fail to open folder %s\n", inPath);
//         exit(EXIT_FAILURE);
//     }
//     while(entry = readdir(dir))
//     {
//         name    =   (char*)malloc(MAX_PATH_LEN);
//         strcpy(name, entry->d_name);
//         PUT_3_STRS_TOGETHER(filePath, inPath, "/", entry->d_name);
//         stat(filePath, &stbuf);
//         fp  =   fopen(filePath, "rb");
//         buf =   (uint8_t*)malloc(stbuf.st_size);
//         if(1 != fread(buf, stbuf.st_size, 1, fp)) ;
//         fclose(fp);

//         if(CHECK_POSTFIX(entry->d_name, "sid"))
//         {
//             readData    =   (de_readPtr)malloc(sizeof(de_readNode));
//             name[strlen(name)-4] = '\0';
//             readData->name  =   name;
//             ptr =   buf;
//             readData->basename_and_oriptr   =   ptr;
//             ptr +=  (strlen(readData->basename_and_oriptr) + 1);
//             readData->sizes  =   (uint32_t*)ptr;
//             ptr +=  13*sizeof(uint32_t);
//             readData->ffxx  =   *ptr++;
//             readData->xx    =   *ptr++;
//             readData->header    =   ptr;
//             ptr +=  readData->sizes[7];
//             #ifdef COMPRESS_DELTA_INS
//             readData->flag  =   *ptr++;
//             #endif
//             readData->x =   ptr;
//             ptr +=  readData->sizes[8];
//             readData->y =   ptr;
//             ptr +=  readData->sizes[9];
//             readData->cp_l  =   ptr;
//             ptr +=  readData->sizes[10];
//             readData->in_l  =   ptr;
//             ptr +=  readData->sizes[11];
//             readData->in_d  =   ptr;
//             readData->next  =   NULL;

//             pthread_mutex_lock(&readList->mutex);
//             if(readList->head)
//                 ((de_readPtr)readList->tail)->next  =   readData;
//             else 
//                 readList->head  =   readData;
//             readList->tail  =   readData;
//             readList->counter   ++;
//             pthread_cond_signal(&readList->rCond);
//             // if(readList->counter == READ_LIST_LEN)
//             //     pthread_cond_wait(&readList->wCond, &readList->mutex);
//             pthread_mutex_unlock(&readList->mutex);
//         }
//         else if(CHECK_POSTFIX(entry->d_name, "jpg") || CHECK_POSTFIX(entry->d_name, "jpeg") || CHECK_POSTFIX(entry->d_name, "JPEG"))
//         {
//             coe =   get_base_coe_mem(buf, stbuf.st_size);
//             if(coe)
//             {
//                 pthread_mutex_lock(&mutex);
//                 g_hash_table_insert(coeTab, name, coe);
//                 pthread_mutex_unlock(&mutex);
//                 *returnSize +=  stbuf.st_size;
//                 #ifndef HEADER_DELTA
//                 free(buf);
//                 #endif
//             }
//             else
//             {
//                 free(name);
//                 free(buf);
//             }
//         }
//         else 
//         {
//             free(name);
//             free(buf);
//         }
//     }
//     closedir(dir);

//     pthread_mutex_lock(&readList->mutex);
//     readList->ending    =   1;
//     pthread_mutex_unlock(&readList->mutex);
//     pthread_cond_signal(&readList->rCond);

//     return (void*)returnSize;
// }

// void free_hashVal(gpointer p)
// {
//     jpeg_coe_ptr    jcp =   (jpeg_coe_ptr)p;
//     free(jcp->data);
//     #ifdef HEADER_DELTA
//     free(jcp->header);
//     #endif
//     free(jcp);
// }

// void free_hashKey(gpointer p)
// {
//     free(p);
// }

// uint64_t idedup_decompress(char *inFolder, char *outFolder
//     #ifdef  CHECK_DECOMPRESS
//         , char *oriFolder
//     #endif
// )
// {
//     GHashTable  *coeTable   =   g_hash_table_new_full(g_str_hash,g_str_equal,free_hashKey, free_hashVal);
//     List        tableList   =   (List)malloc(sizeof(ListNode));
//     tableList->head =   coeTable;
//     pthread_mutex_init(&tableList->mutex,NULL);
//     uint64_t    avaiSize    =   0;

//     List        readList, decdList, deupList;
//     INIT_LIST(readList, 1<<30);
//     INIT_LIST(decdList, 1<<30);
//     pthread_t   read_t_id, decd_t_id;
//     void        *read_arg[] =   {inFolder, readList, tableList};
//     void        *decd_arg[] =   {readList, decdList};
//     void        *rawSize;
//     pthread_create(&read_t_id, NULL, de_read_thread, (void*)read_arg);
//     pthread_create(&decd_t_id, NULL, de_decode_thread, (void*)decd_arg);
//     pthread_join(read_t_id, (void**)(&rawSize));
//     pthread_join(decd_t_id, NULL);
//     avaiSize    +=  *((uint64_t*)rawSize);
//     free(rawSize);

//     INIT_LIST(deupList, 1<<30);
//     pthread_t   deup_t_id, enco_t_id;
//     void        *deup_arg[] =   {decdList, deupList, tableList};
//     void        *enco_arg[] =   {deupList, outFolder
//                                     #ifdef  CHECK_DECOMPRESS
//                                         , oriFolder
//                                     #endif
//                                 };
//     void        *restSize;
//     pthread_create(&deup_t_id, NULL, de_dedup_thread, (void*)deup_arg);
//     pthread_create(&enco_t_id, NULL, de_encode_and_write_thread, (void*)enco_arg);
//     pthread_join(deup_t_id, NULL);
//     pthread_join(enco_t_id, (void**)(&restSize));
//     avaiSize    +=  *((uint64_t*)restSize);
//     free(restSize);

//     DESTROY_LIST(readList);
//     DESTROY_LIST(decdList);
//     DESTROY_LIST(deupList);

//     pthread_mutex_destroy(&tableList->mutex);
//     g_hash_table_destroy(coeTable);
//     free(tableList);

//     return avaiSize;
// }


/////////////////////////////------------------------------------------/////////////


static void de_free_node(gpointer p)
{
    buf_node    *node   =   (buf_node*)p;
    de_node     *de_node_data   =   (de_node*)node->data;

    if(de_node_data->coe)
    {
        free(de_node_data->coe->data);
        free(de_node_data->coe->header);
        free(de_node_data->coe);
    }
    free(de_node_data->folder);
    free(de_node_data->name);
    free(de_node_data);
    pthread_mutex_destroy(&node->mutex);
    free(node);
}

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

static uint64_t fill_buf_node(void *p)
{
    buf_node    *node   =   (buf_node*)p;
    de_node     *de_node_data   =   (de_node*)node->data;
    if(de_node_data->coe == NULL)
    {
        char    fileName[MAX_PATH_LEN];
        PUT_3_STRS_TOGETHER(fileName, de_node_data->folder, "/", de_node_data->name);
        struct stat stbuf;
        stat(fileName, &stbuf);
        uint8_t *tmp    =   (uint8_t*)malloc(stbuf.st_size);
        FILE    *fp =   fopen(fileName, "rb");//这个文件夹要改一下，对于target而言文件夹应该是rst
        fread(tmp, stbuf.st_size, 1, fp);
        fclose(fp);
        de_node_data->coe   =   get_base_coe_mem(tmp, stbuf.st_size);
        return node->size;
    }
    return 0;
}

void* de_read_thread(void *parameter)
{
    void    **arg       =   (void**)parameter;
    uint8_t **data      =   (uint8_t**)arg[0];
    List    readList    =   (List)arg[1];
    GHashTable  *imgTab =   (GHashTable*)arg[2];
    pthread_mutex_t     *dataMutex  =   (pthread_mutex_t*)arg[3], 
                        *tabMutex   =   (pthread_mutex_t*)arg[4];
    uint8_t *ptr, *boundary =   (uint8_t*)arg[5];
    Buffer  *buf        =   (Buffer*)arg[6];
    char    *inFolder   =   (char*)arg[7];
    List    middleList  =   (List)arg[8];
    FILE        *fp;
    buf_node    *base;
    List        waitList;
    INIT_LIST(waitList, 0);
    de_readPtr  readPtr, procPtr = NULL, waitPre = waitList->head, waitNext, headPtr;
    #ifdef PART_TIME
    GTimer  *timer  =   g_timer_new();
    #endif

    waitList->head  =   g_malloc0(sizeof(de_readNode));
    headPtr =   waitList->head;

    #ifdef PART_TIME
    g_timer_start(timer);
    #endif

    while(1)
    {
        pthread_mutex_lock(dataMutex);
        if(*data < boundary)
        {
            readPtr =   (de_readPtr)malloc(sizeof(de_readNode));
            ptr     =   *data;
            readPtr->basename_and_oriptr =   ptr;
            readPtr->name    =   ptr + strlen(readPtr->basename_and_oriptr) + 1;
            ptr     =   readPtr->name + strlen(readPtr->name) + 1;
            readPtr->sizes   =   (uint32_t*)ptr;
            ptr     +=  sizeof(uint32_t)*13;
            readPtr->ffxx    =   *ptr++;
            readPtr->xx      =   *ptr++;
            readPtr->header  =   ptr;
            ptr     +=  readPtr->sizes[7];
            readPtr->flag    =   *ptr++;
            readPtr->x       =   ptr;
            readPtr->y       =   readPtr->x + readPtr->sizes[8];
            readPtr->cp_l    =   readPtr->y + readPtr->sizes[9];
            readPtr->in_l    =   readPtr->cp_l + readPtr->sizes[10];
            readPtr->in_d    =   readPtr->in_l + readPtr->sizes[11];
            readPtr->mem_size=   readPtr->in_d - *data + readPtr->sizes[12];
            readPtr->next    =   NULL;
            *data   =   readPtr->in_d + readPtr->sizes[12];
        }
        pthread_mutex_unlock(dataMutex);

        if(readPtr)
        {
            readPtr->next   =   headPtr->next;
            headPtr->next   =   readPtr;
            waitList->counter   ++;
            // procPtr =   headPtr->next;
            waitPre =   headPtr;
        }
        else
        {
            if(waitList->counter)
            {
                readPtr =   headPtr->next;
                waitPre =   headPtr;
                // printf("%s-----%s\n",readPtr->name,readPtr->basename_and_oriptr);
            }
            else
            break;
        }

        while(readPtr)
        {
            waitNext    =   readPtr->next;

            pthread_mutex_lock(tabMutex);
            if((base = g_hash_table_lookup(imgTab, readPtr->basename_and_oriptr)) == NULL)
            {
                char    fileName[MAX_PATH_LEN];
                PUT_3_STRS_TOGETHER(fileName, inFolder, "/", readPtr->basename_and_oriptr);

                if(fp = fopen(fileName, "rb"))
                {
                    waitPre->next   =   readPtr->next;
                    waitList->counter   --;
                    
                    buf_node    *node = (buf_node*)malloc(sizeof(buf_node));
                    de_node *node_data = (de_node*)malloc(sizeof(de_node));
                    node_data->name =   malloc(strlen(readPtr->basename_and_oriptr)+1);
                    node_data->folder   =   malloc(strlen(inFolder)+1);
                    strcpy(node_data->name, readPtr->basename_and_oriptr);
                    strcpy(node_data->folder, inFolder);
                    pthread_mutex_init(&node->mutex, NULL);
                    node->data  =   node_data;
                    node->link  =   1;
                    pthread_mutex_lock(&node->mutex);
                    g_hash_table_insert(imgTab, node_data->name, node);
                    pthread_mutex_unlock(tabMutex);

                    struct stat stbuf;
                    stat(fileName, &stbuf);
                    uint8_t *tmp = (uint8_t*)malloc(stbuf.st_size);
                    fread(tmp, stbuf.st_size, 1, fp);
                    fclose(fp);
                    node_data->coe  =   (jpeg_coe_ptr)tmp;

                    node->size  =   stbuf.st_size;

                    pairPtr pair = (pairPtr)malloc(sizeof(pairData));
                    pair->target    =   readPtr;
                    pair->base      =   node;
                    pair->mem_size  =   sizeof(pairData) + sizeof(buf_node) + sizeof(de_readNode) +
                                        node->size + readPtr->mem_size;

                    pthread_mutex_lock(&middleList->mutex);
                    while(middleList->size < pair->mem_size)
                        pthread_cond_wait(&middleList->wCond, &middleList->mutex);
                    if(middleList->counter)
                        ((pairPtr)middleList->tail)->next =   pair;
                    else
                        middleList->head    =   pair;
                    middleList->tail    =   pair;
                    middleList->counter ++;
                    middleList->size    -=  pair->mem_size;
                    pthread_cond_signal(&middleList->rCond);
                    pthread_mutex_unlock(&middleList->mutex);
                }
                else
                {
                    pthread_mutex_unlock(tabMutex);
                    waitPre =   readPtr;
                }
            }
            else
            {
                waitPre->next =   readPtr->next;
                waitList->counter   --;

                pthread_mutex_unlock(tabMutex);
                pthread_mutex_lock(&base->mutex);
                base->link ++;
                pthread_mutex_unlock(&base->mutex);
                move_in_buffer(base, buf, fill_buf_node, free_buf_node);

                pairPtr pair = (pairPtr)malloc(sizeof(pairData));
                pair->target    =   readPtr;
                pair->base      =   base;
                pair->mem_size  =   sizeof(pairData) + sizeof(buf_node) + sizeof(de_readNode) +
                                    base->size + readPtr->mem_size;

                pthread_mutex_lock(&readList->mutex);
                while(readList->size < pair->mem_size)
                    pthread_cond_wait(&readList->wCond, &readList->mutex);
                if(readList->counter)
                    ((pairPtr)readList->tail)->next =   pair;
                else
                    readList->head    =   pair;
                readList->tail    =   pair;
                readList->counter ++;
                readList->size    -=  pair->mem_size;
                pthread_cond_signal(&readList->rCond);
                pthread_mutex_unlock(&readList->mutex);
            }
            readPtr =   waitNext;
        }
    }

    #ifdef PART_TIME
    pthread_mutex_lock(&read_time_mutex);
    read_time   +=  g_timer_elapsed(timer, NULL);
    pthread_mutex_unlock(&read_time_mutex);
    #endif

    #ifdef PART_TIME
    g_timer_destroy(timer);
    #endif

    DESTROY_LIST(waitList);
    free(headPtr);

    pthread_mutex_lock(&readList->mutex);
    readList->ending    ++;
    // for(int i=0; i<MIDDLE_THREAD_NUM; i++)
        pthread_cond_signal(&readList->rCond);
    pthread_mutex_unlock(&readList->mutex);

    pthread_mutex_lock(&middleList->mutex);
    middleList->ending  ++;
    // for(int i=0; i<MIDDLE_THREAD_NUM; i++)
        pthread_cond_signal(&middleList->rCond);
    pthread_mutex_unlock(&middleList->mutex);
}

void* de_write_thread(void *parameter)
{
    void        **arg   =   (void**)parameter;
    List        writList=   (List)arg[0];
    char        *folder =   (char*)arg[1];
    encodedDataPtr  encdPtr;
    #ifdef  PART_TIME
    GTimer      *timer  =   g_timer_new();
    #endif

    while(1)
    {
        pthread_mutex_lock(&writList->mutex);
        while(writList->counter == 0)
        {
            if(writList->ending == MIDDLE_THREAD_NUM)   goto ESCAPE_LOOP;
            pthread_cond_wait(&writList->rCond, &writList->mutex);
        }
        encdPtr =   writList->head;
        writList->head  =   encdPtr->next;
        writList->size  +=  encdPtr->mem_size;
        writList->counter   --;
        pthread_cond_signal(&writList->wCond);
        pthread_mutex_unlock(&writList->mutex);

        #ifdef PART_TIME
        g_timer_start(timer);
        #endif

        char    outPath[MAX_PATH_LEN];
        PUT_3_STRS_TOGETHER(outPath, folder, "/", encdPtr->name);
        FILE    *fp =   fopen(outPath, "wb");
        fwrite(encdPtr->data, encdPtr->size, 1, fp);
        // fflush(fp);
        // fsync(fileno(fp));
        fclose(fp);

        free(encdPtr->data);
        free(encdPtr->name);
        free(encdPtr);

        #ifdef PART_TIME
        pthread_mutex_lock(&write_time_mutex);
        write_time   +=  g_timer_elapsed(timer, NULL);
        pthread_mutex_unlock(&write_time_mutex);
        #endif
    }

    #ifdef PART_TIME
    g_timer_destroy(timer);
    #endif

    ESCAPE_LOOP:
    pthread_mutex_unlock(&writList->mutex);
}

uint64_t* idedup_decompress(char *inFolder, char *outFolder, char *idpPath
    #ifdef  CHECK_DECOMPRESS
    , char *oriFolder
    #endif
)
{
    uint64_t    *reVal  =   (uint64_t*)g_malloc0(sizeof(uint64_t)*2);
    #ifdef  PART_TIME
    GTimer      *timer  =   g_timer_new();
    g_timer_start(timer);
    #endif
    FILE    *fp =   fopen(idpPath, "rb");
    struct  stat    statbuf;
    stat(idpPath, &statbuf);
    uint64_t    idpSize =   statbuf.st_size;
    uint8_t     *data   =   (uint8_t*)malloc(idpSize);
    fread(data, idpSize, 1, fp);
    fclose(fp);
    #ifdef PART_TIME
    pthread_mutex_lock(&read_time_mutex);
    read_time   +=  g_timer_elapsed(timer, NULL);
    pthread_mutex_unlock(&read_time_mutex);
    g_timer_destroy(timer);
    #endif
    uint8_t     **pointer   =   (uint8_t**)malloc(sizeof(uint8_t*)), *boundary  =   data + idpSize;
    *pointer    =   data;

    GHashTable  *imageTable =   g_hash_table_new_full(g_str_hash,g_str_equal,NULL,de_free_node);

    Buffer      imgBuf;
    imgBuf.head =   NULL;
    imgBuf.tail =   NULL;
    imgBuf.size =   DECODE_BUFFER_SIZE;
    pthread_mutex_init(&imgBuf.mutex, NULL);
    
    pthread_mutex_t imgTabMutex, dataMutex;
    pthread_mutex_init(&imgTabMutex, NULL);
    pthread_mutex_init(&dataMutex, NULL);
    List    readList[MIDDLE_THREAD_NUM], jpegList[MIDDLE_THREAD_NUM], decodeList[MIDDLE_THREAD_NUM],
            dedupList[MIDDLE_THREAD_NUM] ,writeList;
    for(int i=0; i<MIDDLE_THREAD_NUM; i++)
    {
        INIT_LIST(readList[i], READ_LIST_MAX);
        INIT_LIST(jpegList[i], INDX_LIST_MAX);
        INIT_LIST(decodeList[i], DECD_LIST_MAX);
        INIT_LIST(dedupList[i], DEUP_LIST_MAX);
    }
    INIT_LIST(writeList, REJG_LIST_MAX);

    pthread_t   read_t_id[MIDDLE_THREAD_NUM], jpeg_t_id[MIDDLE_THREAD_NUM], decd_t_id[MIDDLE_THREAD_NUM],
                deup_t_id[MIDDLE_THREAD_NUM], encd_t_id[MIDDLE_THREAD_NUM], writ_t_id;
    void        **read_arg[MIDDLE_THREAD_NUM], **jpeg_arg[MIDDLE_THREAD_NUM], **decd_arg[MIDDLE_THREAD_NUM],
                **deup_arg[MIDDLE_THREAD_NUM], **encd_arg[MIDDLE_THREAD_NUM], *writ_arg[] = {writeList, outFolder};
    for(int i=0; i<MIDDLE_THREAD_NUM; i++)
    {
        read_arg[i] = (void**)malloc(sizeof(void*)*9);
        read_arg[i][0] = pointer;
        read_arg[i][1] = readList[i];
        read_arg[i][2] = imageTable;
        read_arg[i][3] = &dataMutex;
        read_arg[i][4] = &imgTabMutex;
        read_arg[i][5] = boundary;
        read_arg[i][6] = &imgBuf;
        read_arg[i][7] = inFolder;
        read_arg[i][8] = jpegList[i];
        pthread_create(&read_t_id[i], NULL, de_read_thread, (void*)read_arg[i]);

        jpeg_arg[i] = (void**)malloc(sizeof(void*)*3);
        jpeg_arg[i][0] = jpegList[i];
        jpeg_arg[i][1] = readList[i];
        jpeg_arg[i][2] = &imgBuf;
        pthread_create(&jpeg_t_id[i], NULL, de_middle_thread, (void*)jpeg_arg[i]);

        decd_arg[i] = (void**)malloc(sizeof(void*)*2);
        decd_arg[i][0] = readList[i];
        decd_arg[i][1] = decodeList[i];
        pthread_create(&decd_t_id[i], NULL, de_decode_thread, (void*)decd_arg[i]);

        deup_arg[i] = (void**)malloc(sizeof(void*)*5);
        deup_arg[i][0] = decodeList[i];
        deup_arg[i][1] = dedupList[i];
        deup_arg[i][2] = &imgTabMutex;
        deup_arg[i][3] = imageTable;
        deup_arg[i][4] = &imgBuf;
        pthread_create(&deup_t_id[i], NULL, de_dedup_thread, (void*)deup_arg[i]);

        #ifdef  CHECK_DECOMPRESS
        encd_arg[i] = (void**)malloc(sizeof(void*)*3);
        encd_arg[i][0] = dedupList[i];
        encd_arg[i][1] = writeList;
        encd_arg[i][2] = oriFolder;
        #else
        encd_arg[i] = (void**)malloc(sizeof(void*)*2);
        encd_arg[i][0] = dedupList[i];
        encd_arg[i][1] = writeList;
        #endif
        pthread_create(&encd_t_id[i], NULL, de_encode_thread, (void*)encd_arg[i]);
    }
    pthread_create(&writ_t_id, NULL, de_write_thread, (void*)writ_arg);
    

    for(int i=0; i<MIDDLE_THREAD_NUM; i++)
    {
        pthread_join(read_t_id[i], NULL);
        free(read_arg[i]);
    }
    for(int i=0; i<MIDDLE_THREAD_NUM; i++)
    {
        pthread_join(jpeg_t_id[i], NULL);
        free(jpeg_arg[i]);
    }
    for(int i=0; i<MIDDLE_THREAD_NUM; i++)
    {
        pthread_join(decd_t_id[i], NULL);
        free(decd_arg[i]);
    }
    for(int i=0; i<MIDDLE_THREAD_NUM; i++)
    {
        pthread_join(deup_t_id[i], NULL);
        free(deup_arg[i]);
    }
    void    *encdReturn[MIDDLE_THREAD_NUM];
    for(int i=0; i<MIDDLE_THREAD_NUM; i++)
    {
        pthread_join(encd_t_id[i], (void**)(&encdReturn[i]));
        free(encd_arg[i]);
        reVal[0] += ((uint64_t*)encdReturn[i])[0];
        reVal[1] += ((uint64_t*)encdReturn[i])[1];
        free(encdReturn[i]);
    }
    pthread_join(writ_t_id, NULL);

    for(int i=0; i<MIDDLE_THREAD_NUM; i++)
    {
        DESTROY_LIST(readList[i]);
        DESTROY_LIST(jpegList[i]);
        DESTROY_LIST(decodeList[i]);
        DESTROY_LIST(dedupList[i]);
    }
    DESTROY_LIST(writeList);

    pthread_mutex_destroy(&imgTabMutex);
    pthread_mutex_destroy(&dataMutex);
    pthread_mutex_destroy(&imgBuf.mutex);

    g_hash_table_destroy(imageTable);
    free(data);
    free(pointer);

    return reVal;
}