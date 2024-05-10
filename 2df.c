/*
 * @Author: Cai Deng
 * @Date: 2020-11-19 11:32:09
 * @LastEditors: Cai Deng dengcaidengcai@163.com
 * @LastEditTime: 2024-05-10 07:53:09
 * @Description: 
 */

#include "2df.h"
#include "idelta.h"
#include "rabin/rabin.h"

extern uint8_t FEATURE_METHOD;
extern int _block_size;
extern uint8_t one_dimension;
extern uint8_t data_type;
#ifdef CHECK_BUFFER
extern struct buffer_counter {
    uint64_t    counter;
    pthread_mutex_t mutex;
}   request, miss;
#endif
#ifdef COLLISION_RATE
extern struct collision_counter {
    uint64_t    counter;
    pthread_mutex_t mutex;
}   query, collision;
#endif
#ifdef PART_TIME
extern double detect_time;
extern pthread_mutex_t detect_time_mutex;
#endif

extern uint64_t out_table_f[256], mod_table_f[256];

uint64_t k_index[] = {
    0x76931fac9dab2b36, 0xc248b87d6ae33f9a, 0x62d7183a5d5789e4, 0xb2d6b441e2411dc7, 
    0x09e111c7e1e7acb6, 0xf8cac0bb2fc4c8bc, 0x2ae3baaab9165cc4, 0x58e199cb89f51b13, 
    0x5f7091a5abb0874d, 0xf3e8cb4543a5eb93, 0xb0441e9ca4c2b0fb, 0x3d30875cbf29abd5, 
    0xb1acf38984b35ae8, 0x82809dd4cfe7abc5, 0xc61baa52e053b4c3, 0x643f204ef259d2e9, 
    0x8042a948aac5e884, 0xcb3ec7db925643fd, 0x34fdd467e2cca406, 0x035cb2744cb90a63, 
    0xe51c973790334394, 0x7e02086541e4c48a, 0x99630aa9aece1538, 0x43a4b190274ebc95, 
    0x5f8592e30a2205a4, 0x85846248987550aa, 0xf2094ec59e7931dc, 0x650c7451cc61c0cb, 
    0x2c46a1b3f2c349fa, 0xff763c7f8d14ddff, 0x946351744378d62c, 0x59285a8d7915614f, 
    0x5a2ac9e0d68aca62, 0x48a9227ab8f1930e, 0xe38ac7a9d239c9b0, 0x26a481e49d53161f, 
    0x9a9513fe5271c32e, 0x9c21d156eb9f1bea, 0x57f6ae4f1b1de3b7, 0xfd9cee2d9cca7b4c, 
    0x242d26c31d000b7f, 0x90b7fe48a131c7de, 0xbfbe58165266de56, 0xe1edf26939af07ec, 
    0x69ab1b17d8db6214, 0x3f2228b51551c3d2, 0xc7de3f5072bd4d18, 0xc3aeb64cb9e8cba8,
    0x1a0f3783ef9012db, 0x00a903566bce3501, 0xd2223908bccfe509, 0x5903acde8fd7ab31,
    0x935db607ea31258f, 0xe90788fdac21bd00, 0x235ad90b73c1e502, 0xe547f90ac56b73a2,
    0xa9073451a897d342, 0xc1d23f55690bb5a1, 0x3392b830b514a6f5, 0x6aaa890d35f0ff59,
    0x763fcba8bd62469f, 0x4fdb4529602ad675, 0x8f8263b034fadbc7, 0xf83bd098236ac562
};

uint64_t b_index[] = {
    0x38667b6ed2b2fcab, 0x04abae8676e318b4, 0x02a7d15b30d2d7dd, 0xb78650cc6af82bc3, 
    0xd7aa805b02dd9aa5, 0x23b7374a1323ee6b, 0x516d1b81e5f709c2, 0xc790edaf1c3fa9b0, 
    0xa1dbc6dabc2b5ed2, 0x67244c458752002b, 0x106d6381fad58a7e, 0x193657bde0fe0291, 
    0x20f8379316891f82, 0x8b8d24a049e5b86d, 0x855bcfed56765f9d, 0xa1ac54caeaf9257a, 
    0xbc67b451bc70b0e5, 0x2817dd1b704a6b41, 0x8a83fd4a9ca4c89e, 0x1a6e779f8d9e9df1, 
    0x8747591e5b314c05, 0x763edcd59632423c, 0xa83f14d6f073d784, 0xdb2b7001643a6760, 
    0xf9f0dd6ddd0a59e2, 0x41dc1ed720287896, 0x286f5cc3addf6c1a, 0xdf6ed35f477b0022, 
    0x981e5e1fbfe1bfb8, 0xe26b5ba93253275b, 0xf6a44b3fa1051cdf, 0xe3b3f5d2725a9a58, 
    0x0fd5b04525b3182f, 0xcd2b3fda124aca3c, 0x901406a2b55cd8b9, 0x5d48d13e379f1ccb, 
    0xcdfc39fee4acc552, 0x3aa0bdef57e63a1f, 0x81cbaba9f45caaed, 0x48d06bfb3d168360, 
    0x42bed57cac84761b, 0xfeb59a0c81304908, 0xbb781e4bbdf230d2, 0xe977374b97bd0b6b, 
    0x7d38b736428826a0, 0xf2729be2290256dc, 0x304e875c9d4b3fb2, 0x125ae3d0cd3130d6,
    0x3764bdca939cad56, 0x290bfd3ea9c74cbe, 0xcb32a05648982795, 0xb2083afde0219374,
    0x09389bfad721f43d, 0x458475badc30a38d, 0xbad72854902bd01a, 0xcf81993a3acb4302,
    0xf4b8eac294a96d54, 0x18321da9c9410111, 0x00df012104bc0103, 0x110018201acdf900,
    0xcc490ab371f1138f, 0x9327ad39875abef4, 0xabbb29843297f091, 0x0932998100000ac0
};

static void free_node(gpointer p)
{
    buf_node    *node   =   (buf_node*)p;
    imagePtr    image   =   (imagePtr)node->data;

    decodedDataPtr  decodeptr   =   image->decdData;

        rawDataPtr  rawPtr  =   decodeptr->rawData;
        free(rawPtr->name);
        free(rawPtr->dir_name);
        if(rawPtr->data)
            free(rawPtr->data);
        free(rawPtr);

            jpeg_coe_ptr    coe =   decodeptr->targetInfo->coe;
            if(coe)
            {
                free(coe->data);
                free(coe);
            }
        free(decodeptr->targetInfo);

    free(decodeptr);

    free(image->sfs);
    #ifdef FIX_OPTI
    free(image->position);
    #endif
    #ifdef FEATURE_CHECK
    free(image->dc);
    #endif
    free(image);
    pthread_mutex_destroy(&node->mutex);
    free(node);
}

static void free_buf_node(void *p)
{
    imagePtr    image   =   (imagePtr)p;
    free(image->decdData->rawData->data);
    image->decdData->rawData->data  =   NULL;
    jpeg_coe_ptr    ptr =   image->decdData->targetInfo->coe;
    free(ptr->data);
    free(ptr);
    image->decdData->targetInfo->coe    =   NULL;
}

static uint64_t fill_buf_node(void *p)
{
    #ifdef CHECK_BUFFER
    pthread_mutex_lock(&request.mutex);
    request.counter ++;
    pthread_mutex_unlock(&request.mutex);
    #endif
    buf_node    *node   =   (buf_node*)p;
    decodedDataPtr  decode  =   ((imagePtr)node->data)->decdData;
    target_ptr  ptr     =   decode->targetInfo;
    if(ptr->coe == NULL)
    {
        #ifdef CHECK_BUFFER
        pthread_mutex_lock(&miss.mutex);
        miss.counter ++;
        pthread_mutex_unlock(&miss.mutex);
        #endif
        rawDataPtr  rawPtr  =   decode->rawData;
        char    fileName[MAX_PATH_LEN];
        PUT_3_STRS_TOGETHER(fileName, rawPtr->dir_name, "/", rawPtr->name);
        FILE    *fp     =   fopen(fileName, "rb");
        uint8_t *tmp    =   (uint8_t*)malloc(rawPtr->size);
        if(fread(tmp, rawPtr->size, 1, fp)!=1) ;
        fclose(fp);
        rawPtr->data    =   tmp;
        ptr->coe    =   get_base_coe_mem(tmp, rawPtr->size);
        return  node->size;
    }
    return 0;
}

static imagePtr compute_features(decodedDataPtr decodePtr)
{
    int         i, j, k, m;
    imagePtr    image   =   (imagePtr)malloc(sizeof(imageData));
                image->sfs  =   (uint64_t*)malloc(sizeof(uint64_t)*SF_NUM);
                #ifdef FIX_OPTI
                image->position =   (uint64_t*)malloc(sizeof(uint64_t)*SF_NUM);
                #endif
                #ifdef FEATURE_CHECK
                image->dc   =   (short*)malloc(sizeof(short)*SF_NUM);
                #endif
    uint32_t    w       =   decodePtr->targetInfo->coe->imgSize[0],
                h       =   decodePtr->targetInfo->coe->imgSize[1];
    JBLOCKROW   jbrow[h];
    uint8_t     *ptr    =   decodePtr->targetInfo->coe->data;
    for(i=0; i<h; i++, ptr+=sizeof(JBLOCK)*w)
        jbrow[i]    =   (JBLOCKROW)ptr;
    JBLOCKARRAY jbarray =   jbrow;

    uint64_t    max[FEATURE_NUM], pos[FEATURE_NUM];
    memset(max, 0, FEATURE_NUM*sizeof(uint64_t));
    memset(pos, 0, FEATURE_NUM*sizeof(uint64_t));
    #ifdef FEATURE_CHECK
    short       dc[FEATURE_NUM];
    memset(dc, 0, sizeof(short)*FEATURE_NUM);
    #endif
    uint64_t    ltFeature;
    uint8_t     bit_per_slot = (_block_size < 8)? (64/_block_size/_block_size):1;

    uint64_t    *mapSpace = (uint64_t*)g_malloc0(sizeof(uint64_t)*(w+1)*(h+1));
    uint64_t    *mapPtr = mapSpace;
    uint64_t    **map = (uint64_t**)malloc(sizeof(uint64_t*)*(h+1));
    for(i=0; i<h+1; i++,mapPtr+=w+1)
        map[i]  =   mapPtr;

    struct      rabin_t *rabinHash;
    uint64_t    gearHash;

    if(FEATURE_METHOD == _2DF)
    {
        uint64_t    tmpFeature;
        #ifdef UNIFORMLY_SAMPLE
        int step = 64 / bit_per_slot;
        for(i=0; i<h; i++)
        {
            for(j=0; j<w; j++)
                for(k=0; k<64; k+=step)
                    map[i][j]   =   (map[i][j]<<1) | ((jbarray[i][j][k] & 0x1)? 1:0);
        }
        #else
        int step = 8 / _block_size;
        for(i=0; i<h; i++)
        {
            for(j=0; j<w; j++)
            {
                for(k=0; k<step; k++)
                {
                    for(m=0; m<step; m++)
                    {
                        map[i][j]   =   (map[i][j]<<1) | ((jbarray[i][j][k*8+m] & 0x1)? 1:0);
                    }
                }
            }
        }
        #endif
        
        if(!one_dimension)
        {
            for(i=0; i<=h-_block_size; i++)
            {
                tmpFeature = 0;
                for(j=0; j<_block_size; j++)
                {
                    for(k=0; k<_block_size; k++)
                        tmpFeature = (tmpFeature<<bit_per_slot) | map[i+k][j];
                }
                for(j=_block_size; j<=w; j++)
                {
                    for(m=0; m<FEATURE_NUM; m++)
                    {
                        ltFeature = k_index[m]*tmpFeature + b_index[m];
                        if(ltFeature > max[m])
                        {
                            max[m] = ltFeature;
                            #ifdef FIX_OPTI
                            pos[m] = i;
                            pos[m] = (pos[m]<<32)|j;
                            #endif
                            #ifdef FEATURE_CHECK
                            dc[m]   =   jbarray[i][j][0];
                            #endif
                        }
                    }
                    for(k=0; k<_block_size; k++)
                        tmpFeature = (tmpFeature<<bit_per_slot) | map[i+k][j];
                }
            }
        }
        else 
        {
            uint64_t bs2 = _block_size*_block_size;
            for(i=0; i<=h-1; i++)
            {
                tmpFeature = 0;
                for(j=0; j<bs2&&j<w; j++)
                    tmpFeature = (tmpFeature<<bit_per_slot) | map[i][j];
                j = bs2;
                do
                {
                    for(m=0; m<FEATURE_NUM; m++)
                    {
                        ltFeature = k_index[m]*tmpFeature + b_index[m];
                        if(ltFeature > max[m])
                            max[m] = ltFeature;
                    }
                    tmpFeature = (tmpFeature<<bit_per_slot) | map[i][j];
                }while(++j<=w);
            }
        }
    }
    else if(FEATURE_METHOD == _RABIN)
    {
        uint64_t winsize;
        if(!one_dimension)
        {
            winsize  = _block_size*_block_size*sizeof(JBLOCK);
            for(i=0; i<=h-_block_size; i++)
            {
                rabinHash   =   rabin_init(winsize, out_table_f, mod_table_f);
                for(k=0; k<_block_size; k++)
                {
                    for(j=0; j<_block_size; j++)
                        rabin_slide_a_block(rabinHash, (uint8_t*)jbarray[i+j][k], winsize, out_table_f, mod_table_f);
                }
                for(j=_block_size; j<=w; j++)
                {
                    for(m=0; m<FEATURE_NUM; m++)
                    {
                        ltFeature = k_index[m]*rabinHash->digest + b_index[m];
                        if(ltFeature > max[m])
                            max[m] = ltFeature;
                    }
                    for(m=0; m<_block_size; m++)
                        rabin_slide_a_block(rabinHash, (uint8_t*)jbarray[i+m][j], winsize, out_table_f, mod_table_f);
                }
                rabin_free(rabinHash);
            }
        }
        else if(one_dimension == 1)
        {
            winsize = _block_size;
            int totalBytes;
            uint8_t thisByte;
            if(data_type == DECODED)
            {
                totalBytes = w*h*sizeof(JBLOCK);
                ptr =   decodePtr->targetInfo->coe->data;
            }
            else
            {
                totalBytes = decodePtr->rawData->size;
                ptr = decodePtr->rawData->data;
            }
            
            rabinHash   =   rabin_init(winsize, out_table_f, mod_table_f);
            for(int i=0; i<winsize-1; i++)
            {
                thisByte = *ptr++;
                rabin_slide(rabinHash, thisByte, winsize, out_table_f, mod_table_f);
            }
            for(int i=winsize-1; i<totalBytes; i++)
            {
                thisByte = *ptr++;
                rabin_slide(rabinHash, thisByte, winsize, out_table_f, mod_table_f);
                for(m=0; m<FEATURE_NUM; m++)
                {
                    ltFeature = k_index[m]*rabinHash->digest + b_index[m];
                    if(ltFeature > max[m])
                        max[m] = ltFeature;
                }
            }
            rabin_free(rabinHash);
        }
        else
        {
            winsize = _block_size*sizeof(JBLOCK);
            ptr =   decodePtr->targetInfo->coe->data;
            int totalBlock = w*h;
            rabinHash   =   rabin_init(winsize, out_table_f, mod_table_f);
            for(int i=0; i<_block_size-1; i++,ptr+=sizeof(JBLOCK))
                rabin_slide_a_block(rabinHash, ptr, winsize, out_table_f, mod_table_f);
            for(int i=_block_size-1; i<totalBlock; i++,ptr+=sizeof(JBLOCK))
            {
                rabin_slide_a_block(rabinHash, ptr, winsize, out_table_f, mod_table_f);
                for(m=0; m<FEATURE_NUM; m++)
                {
                    ltFeature = k_index[m]*rabinHash->digest + b_index[m];
                    if(ltFeature > max[m])
                        max[m] = ltFeature;
                }
            }
            rabin_free(rabinHash);
        }
    }
    else if(FEATURE_METHOD == _GEAR)
    {
        if(one_dimension == 1)
        {
            gearHash = 0;
            ptr = decodePtr->targetInfo->coe->data;
            int totalBytes = w*h*sizeof(JBLOCK);
            for(int i=0; i<63; i++)
            {
                gearHash = gear_slide(gearHash, *ptr++);
            }
            for(int i=64; i<totalBytes; i++)
            {
                gear_slide(gearHash, *ptr++);
                // if(gearHash & 0x0120003004100080 == 0)
                // {
                for(m=0; m<FEATURE_NUM; m++)
                {
                    ltFeature = k_index[m]*gearHash + b_index[m];
                    if(ltFeature > max[m])
                        max[m] = ltFeature;
                }
                // }
            }
        }
        else if(one_dimension == 2)
        {
            ptr = decodePtr->targetInfo->coe->data;
            int totalBlock = w*h;
            for(int i=0; i<totalBlock; i++, ptr+=sizeof(JBLOCK))
            {
                gearHash = gear_slide_a_block(ptr);
                for(m=0; m<FEATURE_NUM; m++)
                {
                    ltFeature = k_index[m]*gearHash + b_index[m];
                    if(ltFeature > max[m])
                        max[m] = ltFeature;
                }
            }
        }
    }

    free(map); free(mapSpace);

    for(i=0,k=0; i<SF_NUM; i++)
    {
        image->sfs[i]   =   0;
        #ifdef FIX_OPTI
        image->position[i] =   0;
        #endif
        #ifdef FEATURE_CHECK
        image->dc[i]    =   0;
        #endif
        for(j=0; j<FEA_PER_SF; j++,k++)
        {
            image->sfs[i]   +=  max[k];
            #ifdef FIX_OPTI
            image->position[i]  +=  pos[k];
            #endif
            #ifdef FEATURE_CHECK
            image->dc[i]    +=  dc[k];
            #endif
        }
    }
    image->decdData =   decodePtr;

    return image;
}

#define META_SIZE_2DF (sizeof(buf_node) + sizeof(imageData) + sizeof(decodedDataNode) + sizeof(rawDataNode) + \
    sizeof(target_struct) + sizeof(jpeg_coe) )

static detectionDataPtr detect_a_single_img(decodedDataPtr decodePtr, GHashTable **featureT, Buffer *buf
    #if DETECT_THREAD_NUM!=1
        , pthread_mutex_t *ftMutex
    #endif
)
{
    imagePtr        image   =   compute_features(decodePtr), baseImage;
    uint64_t        *features   =   image->sfs;
    GPtrArray       *bases[SF_NUM], *newArray;
    int             i, j, k;
    uint32_t        tmp;
    uint32_t        matchCounter, bestCounter = 0;
    buf_node        *node   =   (buf_node*)malloc(sizeof(buf_node)), *baseNode, *bestMatch = NULL;
    uint64_t        fix_flag, flag_tmp;

    pthread_mutex_init(&node->mutex, NULL);
    node->data  =   image;
    node->link  =   1;
    node->size  =   decodePtr->rawData->size;
    for(i=0; i<3; i++)
        node->size += 
            decodePtr->targetInfo->coe->imgSize[2*i]*decodePtr->targetInfo->coe->imgSize[2*i+1]*sizeof(JBLOCK);
    insert_to_buffer(node, buf, free_buf_node);

    for(i=0; i<SF_NUM; i++)
    {
        #if DETECT_THREAD_NUM!=1
        pthread_mutex_lock(ftMutex+i);
        #endif
        
        bases[i]    =   (GPtrArray*)g_hash_table_lookup(featureT[i], features+i);
        if(bestCounter < SF_NUM && bases[i])
        {
            tmp =   bases[i]->len;
            for(j=0; j<tmp; j++)
            {
                baseNode    =   (buf_node*)g_ptr_array_index(bases[i], j);
                baseImage   =   (imagePtr)(baseNode->data);
                #ifdef FIX_OPTI
                flag_tmp    =   1;
                #endif
                for(k=0,matchCounter=0; k<SF_NUM; k++)
                {
                    if((baseImage->sfs[k]==features[k])
                        #ifdef FEATURE_CHECK
                        && (baseImage->dc[k]==image->dc[k])
                        #endif
                    )
                    {
                        matchCounter ++;
                        #ifdef FIX_OPTI
                        if(baseImage->position[k]==image->position[k])
                            flag_tmp = 0;
                        #endif
                    }
                }
                if(matchCounter > bestCounter)
                {
                    bestCounter =   matchCounter;
                    bestMatch   =   baseNode;
                    #ifdef FIX_OPTI
                    // if((double)flag_tmp/matchCounter > 0.5)
                    //     fix_flag    =   1;
                    // else
                    //     fix_flag    =   0;
                    fix_flag    =   flag_tmp;
                    #endif
                }
                if(matchCounter == SF_NUM)
                    break;
            }
        }
        
        if(bases[i])
        {
            #ifdef COLLISION_RATE
            pthread_mutex_lock(&collision.mutex);
            collision.counter ++;
            pthread_mutex_unlock(&collision.mutex);
            #endif
            g_ptr_array_add(bases[i], node);
        }
        else 
        {
            if(i)   newArray = g_ptr_array_new_full(2, NULL);
            else    newArray = g_ptr_array_new_full(2, free_node);
            g_ptr_array_add(newArray, node);
            g_hash_table_insert(featureT[i], features+i, newArray);
        }

        #if DETECT_THREAD_NUM!=1
        pthread_mutex_unlock(ftMutex+i);
        #endif
    }
    #ifdef COLLISION_RATE
    pthread_mutex_lock(&query.mutex);
    query.counter += SF_NUM;
    pthread_mutex_unlock(&query.mutex);
    #endif

    if(bestMatch)
    {
        detectionDataPtr    detectPtr   =   (detectionDataPtr)malloc(sizeof(detectionNode));
        detectPtr->base     =   bestMatch;
        detectPtr->target   =   node;
        #ifdef FIX_OPTI
        detectPtr->fix_flag =   fix_flag;
        #endif

        pthread_mutex_lock(&bestMatch->mutex);
        bestMatch->link  ++;
        pthread_mutex_unlock(&bestMatch->mutex);
        move_in_buffer(bestMatch, buf, fill_buf_node, free_buf_node);
        
        return  detectPtr;
    }

    pthread_mutex_lock(&node->mutex);
    node->link  --;
    pthread_mutex_unlock(&node->mutex);
    return  NULL;
}

void* detect_thread(void *parameter)
{
    void        **arg       =   (void**)parameter;
    List        decodeList  =   (List)arg[0];
    List        detectList  =   (List)arg[1];
    char        *outPath    =   (char*)arg[2];
    GHashTable  **featureT  =   (GHashTable**)arg[3];
    pthread_mutex_t *ftMutex=   (pthread_mutex_t*)arg[4];
    Buffer      *decodeBuf  =   (Buffer*)arg[5];
    decodedDataPtr      decodePtr;
    detectionDataPtr    detectPtr;

    uint64_t    *unhandledSize   =   (uint64_t*)g_malloc0(sizeof(uint64_t)*2);

    #ifdef PART_TIME
    GTimer      *timer  =   g_timer_new();
    #endif

    while(1)
    {
        pthread_mutex_lock(&decodeList->mutex);
        while(decodeList->counter == 0)
        {
            if(decodeList->ending == DECODE_THREAD_NUM) goto ESCAPE_LOOP;
            pthread_cond_wait(&decodeList->rCond, &decodeList->mutex);
        }
        decodePtr   =   decodeList->head;
        decodeList->head    =   decodePtr->next;
        decodeList->size    +=  decodePtr->mem_size;
        decodeList->counter --;
        pthread_cond_signal(&decodeList->wCond);
        pthread_mutex_unlock(&decodeList->mutex);

        if(decodePtr)
        {
            unhandledSize[1]    +=  decodePtr->targetInfo->coe->imgSize[0]*decodePtr->targetInfo->coe->imgSize[1]*sizeof(JBLOCK);

            #ifdef PART_TIME
            g_timer_start(timer);
            #endif

            detectPtr   =   detect_a_single_img(decodePtr, featureT, decodeBuf
                #if DETECT_THREAD_NUM!=1
                    , ftMutex
                #endif
            );
            // detectPtr   =   NULL;

            #ifdef PART_TIME
            pthread_mutex_lock(&detect_time_mutex);
            detect_time +=  g_timer_elapsed(timer, NULL);
            pthread_mutex_unlock(&detect_time_mutex);
            #endif

            if(detectPtr)
            {
                detectPtr->mem_size     =   sizeof(detectionNode);

                detectPtr->next =   NULL;
                pthread_mutex_lock(&detectList->mutex);
                while(detectList->size < detectPtr->mem_size)
                    pthread_cond_wait(&detectList->wCond, &detectList->mutex);
                if(detectList->counter)
                    ((detectionDataPtr)detectList->tail)->next  =   detectPtr;
                else
                    detectList->head    =   detectPtr;
                detectList->tail    =   detectPtr;
                detectList->counter ++;
                detectList->size    -=  detectPtr->mem_size;
                pthread_cond_signal(&detectList->rCond);
                pthread_mutex_unlock(&detectList->mutex);
            }
            else 
            {
                #ifndef DO_NOT_WRITE
                char    outFilePath[MAX_PATH_LEN];
                PUT_3_STRS_TOGETHER(outFilePath, outPath, "/", decodePtr->rawData->name);
                FILE    *outFp  =   fopen(outFilePath, "wb");
                fwrite(decodePtr->rawData->data, 1, decodePtr->rawData->size, outFp);
                // fflush(outFp);
                // fsync(fileno(outFp));
                fclose(outFp);

                #endif
                *unhandledSize   +=  decodePtr->rawData->size;

                //     free(decodePtr->rawData->data);
                //     free(decodePtr->rawData);
                //     free(decodePtr->targetInfo->coe->data);
                //     free(decodePtr->targetInfo->coe);
                //     free(decodePtr->targetInfo);
            }
        }
    }

    ESCAPE_LOOP:
    pthread_mutex_unlock(&decodeList->mutex);

    #ifdef PART_TIME
    g_timer_destroy(timer);
    #endif

    pthread_mutex_lock(&detectList->mutex);
    detectList->ending ++;
    pthread_cond_signal(&detectList->rCond);
    pthread_mutex_unlock(&detectList->mutex);

    return  (void*)unhandledSize;
}

void* tra_detect_thread(void *parameter)
{
    void        **arg       =   (void**)parameter;
    List        decodeList  =   (List)arg[0];
    List        detectList  =   (List)arg[1];
    char        *outPath    =   (char*)arg[2];
    GHashTable  **featureT  =   (GHashTable**)arg[3];
    pthread_mutex_t *ftMutex=   (pthread_mutex_t*)arg[4];
    Buffer      *decodeBuf  =   (Buffer*)arg[5];
    decodedDataPtr      decodePtr;
    detectionDataPtr    detectPtr;

    uint64_t    *unhandledSize   =   (uint64_t*)g_malloc0(sizeof(uint64_t));

    #ifdef PART_TIME
    GTimer      *timer  =   g_timer_new();
    #endif

    while(1)
    {
        pthread_mutex_lock(&decodeList->mutex);
        while(decodeList->counter == 0)
        {
            if(decodeList->ending) goto ESCAPE_LOOP;
            pthread_cond_wait(&decodeList->rCond, &decodeList->mutex);
        }
        decodePtr   =   decodeList->head;
        decodeList->head    =   decodePtr->next;
        decodeList->size    +=  decodePtr->mem_size;
        decodeList->counter --;
        pthread_cond_signal(&decodeList->wCond);
        pthread_mutex_unlock(&decodeList->mutex);

        if(decodePtr)
        {
            #ifdef PART_TIME
            g_timer_start(timer);
            #endif

            detectPtr   =   detect_a_single_img(decodePtr, featureT, decodeBuf
            #if DETECT_THREAD_NUM!=1
                , ftMutex
            #endif
            );

            #ifdef PART_TIME
            pthread_mutex_lock(&detect_time_mutex);
            detect_time +=  g_timer_elapsed(timer, NULL);
            pthread_mutex_unlock(&detect_time_mutex);
            #endif

            if(detectPtr)
            {
                detectPtr->mem_size     =   sizeof(detectionNode);

                detectPtr->next =   NULL;
                pthread_mutex_lock(&detectList->mutex);
                while(detectList->size < detectPtr->mem_size)
                    pthread_cond_wait(&detectList->wCond, &detectList->mutex);
                if(detectList->counter)
                    ((detectionDataPtr)detectList->tail)->next  =   detectPtr;
                else
                    detectList->head    =   detectPtr;
                detectList->tail    =   detectPtr;
                detectList->counter ++;
                detectList->size    -=  detectPtr->mem_size;
                pthread_cond_signal(&detectList->rCond);
                pthread_mutex_unlock(&detectList->mutex);
            }
            else 
            {
                #ifndef DO_NOT_WRITE
                char    outFilePath[MAX_PATH_LEN];
                PUT_3_STRS_TOGETHER(outFilePath, outPath, "/", decodePtr->rawData->name);
                FILE    *outFp  =   fopen(outFilePath, "wb");
                fwrite(decodePtr->rawData->data, 1, decodePtr->rawData->size, outFp);
                fclose(outFp);
                #endif
                *unhandledSize   +=  decodePtr->rawData->size;
            }
        }
    }

    ESCAPE_LOOP:
    pthread_mutex_unlock(&decodeList->mutex);

    #ifdef PART_TIME
    g_timer_destroy(timer);
    #endif

    pthread_mutex_lock(&detectList->mutex);
    detectList->ending ++;
    pthread_cond_signal(&detectList->rCond);
    pthread_mutex_unlock(&detectList->mutex);

    return  (void*)unhandledSize;
}