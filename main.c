// /*
//  * @Author: Cai Deng
//  * @Date: 2020-10-12 08:11:45
//  * @LastEditors: Cai Deng
//  * @LastEditTime: 2022-05-13 08:58:27
//  * @Description: 
//  */
// #include "idedup.h"
// #include <getopt.h>
// #include "rabin.h"

// #define COMPRESS 1
// #define DECOMPRESS 2

// int READ_THREAD_NUM;
// int DECODE_THREAD_NUM;
// int MIDDLE_THREAD_NUM;
// int WRITE_THREAD_NUM;

// int ROAD_NUM;

// int64_t DECODE_BUFFER_SIZE;
// int64_t PATCH_SIZE;

// int64_t NAME_LIST_MAX;
// int64_t READ_LIST_MAX;
// int64_t DECD_LIST_MAX;
// int64_t DECT_LIST_MAX;
// int64_t INDX_LIST_MAX;
// int64_t DEUP_LIST_MAX;
// int64_t REJG_LIST_MAX;

// uint8_t chunking_mode;
// uint8_t in_chaos;

// int SF_NUM, FEA_PER_SF;
// uint8_t FEATURE_METHOD;
// int _block_size;
// uint8_t one_dimension;

// #define IDELTA 0
// #define XDELTA 1
// uint8_t delta_method;
// uint8_t data_type;

// #ifdef PART_TIME
// double read_time = 0;
// double decode_time = 0;
// double detect_time = 0;
// double index_time  = 0;
// double dedup_time = 0;
// double rejpeg_time = 0;
// double write_time = 0;
// double encode_time = 0;
// pthread_mutex_t detect_time_mutex, index_time_mutex, dedup_time_mutex, read_time_mutex, write_time_mutex, decode_time_mutex, rejpeg_time_mutex, encode_time_mutex;
// #endif

// #ifdef CHECK_BUFFER
// struct buffer_counter {
//     uint64_t    counter;
//     pthread_mutex_t mutex;
// }   request, miss;
// #endif
// #ifdef COLLISION_RATE
// struct collision_counter {
//     uint64_t    counter;
//     pthread_mutex_t mutex;
// }   query, collision;
// #endif

// #ifdef DEBUG_2
// uint64_t sim_counter[20] = {0};
// pthread_mutex_t sim_counter_mutex;
// #endif

// uint64_t out_table_f[256], out_table_i[256], mod_table_f[256], mod_table_i[256];

// #define CHECK_POSTFIX(fileName,lastName)  (!strcmp(lastName,(fileName) + strlen((fileName)) - strlen((lastName))))

// void main(int argc, char *argv[])
// {
//     GTimer      *timer = g_timer_new();
//     double      time;
//     struct      dirent  *entry;
//     DIR         *dir;
//     char        inPath[MAX_PATH_LEN], outPath[MAX_PATH_LEN], idpPath[MAX_PATH_LEN];
//     #ifdef  CHECK_DECOMPRESS
//     char        oriPath[MAX_PATH_LEN];
//     #endif
//     uint64_t    *result, counter = 0, rawSize = 0, undecodeSize = 0, finalSize = 0, decodedSize = 0, deltaSize_raw = 0, deltaSize_decd = 0, detectSize = 0, omitSize = 0;
//     #ifdef DEBUG_1
//     uint64_t    sizes[9] = {0};
//     #endif
//     int         option, mode = 0;
//     char        *input_path, *output_path, *reference_path;
//     static      struct  option  longOptions[]   =   
//     {
//         {"read_thrd_num", required_argument, NULL, 'a'},
//         {"decode_thrd_num", required_argument, NULL, 'B'},
//         {"middle_thrd_num", required_argument, NULL, 'b'},
//         {"write_thrd_num", required_argument, NULL, 'e'},
//         {"input_path", required_argument, NULL, 'f'},
//         {"output_path", required_argument, NULL, 'g'},
//         {"reference_path", required_argument, NULL, 'h'},
//         {"buffer_size", required_argument, NULL, 'i'},
//         {"patch_size", required_argument, NULL, 'j'},
//         {"name_list", required_argument, NULL, 'k'},
//         {"read_list", required_argument, NULL, 'l'},
//         {"decd_list", required_argument, NULL, 'm'},
//         {"dect_list", required_argument, NULL, 'n'},
//         {"indx_list", required_argument, NULL, 'A'},
//         {"deup_list", required_argument, NULL, 'o'},
//         {"rejg_list", required_argument, NULL, 'p'},
//         {"chunking", required_argument, NULL, 'q'},
//         {"road_num", required_argument, NULL, 'r'},
//         {"chaos", required_argument, NULL, 's'},
//         {"sf_num", required_argument, NULL, 't'},
//         {"sf_component_num", required_argument, NULL, 'u'},
//         {"feature_method", required_argument, NULL, 'v'},
//         {"block_size", required_argument, NULL, 'w'},
//         {"dimension", required_argument, NULL, 'x'},
//         {"delta", required_argument, NULL, 'y'},
//         {"data_type", required_argument, NULL, 'z'},
//         {"jpeg_list", required_argument, NULL, 'C'},
//         {"encd_list", required_argument, NULL, 'D'}
//     };

//     while((option = getopt_long_only(argc, argv, "cd", longOptions, NULL))!=-1)
//     {
//         switch (option)
//         {
//         case 'c':
//             mode = COMPRESS;
//             break;
//         case 'd':
//             mode = DECOMPRESS;
//             break;
//         case 'a':
//             READ_THREAD_NUM = atoi(optarg);
//             break;
//         case 'B':
//             DECODE_THREAD_NUM = atoi(optarg);
//             break;
//         case 'b':
//             MIDDLE_THREAD_NUM = atoi(optarg);
//             break;
//         case 'e':
//             WRITE_THREAD_NUM = atoi(optarg);
//             break;
//         case 'f':
//             input_path = optarg;
//             break;
//         case 'g':
//             output_path = optarg;
//             break;
//         case 'h':
//             reference_path = optarg;
//             break;
//         case 'i':
//             if(*optarg=='G' || *optarg=='g')      DECODE_BUFFER_SIZE = ((int64_t)atoi(optarg+1)) << 30;
//             else if(*optarg=='M' || *optarg=='m') DECODE_BUFFER_SIZE = ((int64_t)atoi(optarg+1)) << 20;
//             else if(*optarg=='K' || *optarg=='k') DECODE_BUFFER_SIZE = ((int64_t)atoi(optarg+1)) << 10;
//             else                                  DECODE_BUFFER_SIZE = (int64_t)atoi(optarg);
//             break;
//         case 'j':
//             if(*optarg=='G' || *optarg=='g')      PATCH_SIZE = ((int64_t)atoi(optarg+1)) << 30;
//             else if(*optarg=='M' || *optarg=='m') PATCH_SIZE = ((int64_t)atoi(optarg+1)) << 20;
//             else if(*optarg=='K' || *optarg=='k') PATCH_SIZE = ((int64_t)atoi(optarg+1)) << 10;
//             else                                  PATCH_SIZE = (int64_t)atoi(optarg);
//             break;
//         case 'k':
//             if(*optarg=='G' || *optarg=='g')      NAME_LIST_MAX = ((int64_t)atoi(optarg+1)) << 30;
//             else if(*optarg=='M' || *optarg=='m') NAME_LIST_MAX = ((int64_t)atoi(optarg+1)) << 20;
//             else if(*optarg=='K' || *optarg=='k') NAME_LIST_MAX = ((int64_t)atoi(optarg+1)) << 10;
//             else                                  NAME_LIST_MAX = (int64_t)atoi(optarg);
//             break;
//         case 'l':
//             if(*optarg=='G' || *optarg=='g')      READ_LIST_MAX = ((int64_t)atoi(optarg+1)) << 30;
//             else if(*optarg=='M' || *optarg=='m') READ_LIST_MAX = ((int64_t)atoi(optarg+1)) << 20;
//             else if(*optarg=='K' || *optarg=='k') READ_LIST_MAX = ((int64_t)atoi(optarg+1)) << 10;
//             else                                  READ_LIST_MAX = (int64_t)atoi(optarg);
//             break;
//         case 'm':
//             if(*optarg=='G' || *optarg=='g')      DECD_LIST_MAX = ((int64_t)atoi(optarg+1)) << 30;
//             else if(*optarg=='M' || *optarg=='m') DECD_LIST_MAX = ((int64_t)atoi(optarg+1)) << 20;
//             else if(*optarg=='K' || *optarg=='k') DECD_LIST_MAX = ((int64_t)atoi(optarg+1)) << 10;
//             else                                  DECD_LIST_MAX = (int64_t)atoi(optarg);
//             break;
//         case 'n':
//             if(*optarg=='G' || *optarg=='g')      DECT_LIST_MAX = ((int64_t)atoi(optarg+1)) << 30;
//             else if(*optarg=='M' || *optarg=='m') DECT_LIST_MAX = ((int64_t)atoi(optarg+1)) << 20;
//             else if(*optarg=='K' || *optarg=='k') DECT_LIST_MAX = ((int64_t)atoi(optarg+1)) << 10;
//             else                                  DECT_LIST_MAX = (int64_t)atoi(optarg);
//             break;
//         case 'A':
//             if(*optarg=='G' || *optarg=='g')      INDX_LIST_MAX = ((int64_t)atoi(optarg+1)) << 30;
//             else if(*optarg=='M' || *optarg=='m') INDX_LIST_MAX = ((int64_t)atoi(optarg+1)) << 20;
//             else if(*optarg=='K' || *optarg=='k') INDX_LIST_MAX = ((int64_t)atoi(optarg+1)) << 10;
//             else                                  INDX_LIST_MAX = (int64_t)atoi(optarg);
//             break;
//         case 'o':
//             if(*optarg=='G' || *optarg=='g')      DEUP_LIST_MAX = ((int64_t)atoi(optarg+1)) << 30;
//             else if(*optarg=='M' || *optarg=='m') DEUP_LIST_MAX = ((int64_t)atoi(optarg+1)) << 20;
//             else if(*optarg=='K' || *optarg=='k') DEUP_LIST_MAX = ((int64_t)atoi(optarg+1)) << 10;
//             else                                  DEUP_LIST_MAX = (int64_t)atoi(optarg);
//             break;
//         case 'p':
//             if(*optarg=='G' || *optarg=='g')      REJG_LIST_MAX = ((int64_t)atoi(optarg+1)) << 30;
//             else if(*optarg=='M' || *optarg=='m') REJG_LIST_MAX = ((int64_t)atoi(optarg+1)) << 20;
//             else if(*optarg=='K' || *optarg=='k') REJG_LIST_MAX = ((int64_t)atoi(optarg+1)) << 10;
//             else                                  REJG_LIST_MAX = (int64_t)atoi(optarg);
//             break;
//         case 'q':
//             if(*optarg=='v' || *optarg=='V')        chunking_mode   =   0;
//             else                                    chunking_mode   =   1;
//             break;
//         case 'r':
//             ROAD_NUM    =   atoi(optarg);
//             break;
//         case 's':
//             if(*optarg=='y' || *optarg=='Y')     in_chaos   =   1;
//             else                                 in_chaos   =   0;
//             break;
//         case 't':
//             SF_NUM  =   atoi(optarg);
//             break;
//         case 'u':
//             FEA_PER_SF  =   atoi(optarg);
//             break;
//         case 'v':
//             if(!strcmp(optarg, "rabin"))     FEATURE_METHOD = _RABIN;
//             else if(!strcmp(optarg, "gear")) FEATURE_METHOD = _GEAR;
//             else                             FEATURE_METHOD = _2DF;
//             break;
//         case 'w':
//             _block_size =   atoi(optarg);
//             break;
//         case 'x':
//             if(!strcmp(optarg,"1"))     one_dimension = 1;
//             else if(!strcmp(optarg,"2"))one_dimension = 0;
//             else                        one_dimension = 2;
//             break;
//         case 'y':
//             if(!strcmp(optarg,"xdelta"))    delta_method = XDELTA;
//             else                            delta_method = IDELTA;
//             break;
//         case 'z':
//             if(!strcmp(optarg,"decoded"))   data_type = DECODED;
//             else                            data_type = RAW;
//             break;
//         case 'C':
//             if(*optarg=='G' || *optarg=='g')      INDX_LIST_MAX = ((int64_t)atoi(optarg+1)) << 30;
//             else if(*optarg=='M' || *optarg=='m') INDX_LIST_MAX = ((int64_t)atoi(optarg+1)) << 20;
//             else if(*optarg=='K' || *optarg=='k') INDX_LIST_MAX = ((int64_t)atoi(optarg+1)) << 10;
//             else                                  INDX_LIST_MAX = (int64_t)atoi(optarg);
//             break;
//         case 'D':
//             if(*optarg=='G' || *optarg=='g')      REJG_LIST_MAX = ((int64_t)atoi(optarg+1)) << 30;
//             else if(*optarg=='M' || *optarg=='m') REJG_LIST_MAX = ((int64_t)atoi(optarg+1)) << 20;
//             else if(*optarg=='K' || *optarg=='k') REJG_LIST_MAX = ((int64_t)atoi(optarg+1)) << 10;
//             else                                  REJG_LIST_MAX = (int64_t)atoi(optarg);
//             break;
//         default:
//             break;
//         }
//     }

//     uint64_t rabin_win_size;
//     if(mode == COMPRESS)
//     {
//         if(FEATURE_METHOD == _RABIN)
//         {
//             if(one_dimension == 0)  rabin_win_size = _block_size*_block_size*sizeof(JBLOCK);
//             else if(one_dimension == 1) rabin_win_size = _block_size;
//             else    rabin_win_size = _block_size*sizeof(JBLOCK);
//             calc_tables(rabin_win_size,out_table_f,mod_table_f);
//         }
//         calc_tables(sizeof(JBLOCK),out_table_i,mod_table_i);
//     }

//     // if(_block_size == 1 && FEATURE_METHOD == _2DF)    one_dimension   =   1;

//     #ifdef CHECK_BUFFER
//     request.counter = 0;
//     miss.counter = 0;
//     pthread_mutex_init(&request.mutex, NULL);
//     pthread_mutex_init(&miss.mutex, NULL);
//     #endif
//     #ifdef COLLISION_RATE
//     query.counter = 0;
//     collision.counter = 0;
//     pthread_mutex_init(&query.mutex, NULL);
//     pthread_mutex_init(&collision.mutex, NULL);
//     #endif
//     #ifdef DEBUG_2
//     pthread_mutex_init(&sim_counter_mutex,NULL);
//     #endif
//     #ifdef PART_TIME
//     pthread_mutex_init(&read_time_mutex,NULL);
//     pthread_mutex_init(&decode_time_mutex,NULL);
//     pthread_mutex_init(&detect_time_mutex,NULL);
//     pthread_mutex_init(&index_time_mutex,NULL);
//     pthread_mutex_init(&dedup_time_mutex,NULL);
//     pthread_mutex_init(&rejpeg_time_mutex,NULL);
//     pthread_mutex_init(&write_time_mutex,NULL);
//     pthread_mutex_init(&encode_time_mutex,NULL);
//     #endif

//     g_timer_start(timer);

//     if(mode == COMPRESS)
//     {
//         if(!chunking_mode)  READ_THREAD_NUM =   1;  
//         /* only when it equals to 1, this system can chunk in variable mode. */
//         result  =   idedup_compress(input_path, output_path);
//         rawSize +=  result[0];
//         undecodeSize    +=  result[1];
//         finalSize   +=  result[2];
//         #ifdef DEBUG_1
//         for(int i=0; i<9; i++)
//             sizes[i] += result[3+i];
//         #endif
//         decodedSize +=  result[12];
//         deltaSize_raw += result[13];
//         deltaSize_decd += result[14];
//         detectSize += result[15];
//         omitSize    +=  result[16];

//         free(result);
//     }
//     else if(mode == DECOMPRESS)
//     {
//         dir =   opendir(input_path);
//         while(entry = readdir(dir))
//         {
//             if(!strcmp(entry->d_name,".") || !strcmp(entry->d_name,"..") 
//             // || CHECK_POSTFIX(entry->d_name, "idp")
//             || !CHECK_POSTFIX(entry->d_name, "idp")
//             // || !strcmp(entry->d_name,"24.idp")
//             // || !strcmp(entry->d_name,"16.idp")
//             )
//                 continue ;

//             PUT_3_STRS_TOGETHER(idpPath, input_path, "/", entry->d_name);
//             strcpy(inPath, idpPath);
//             inPath[strlen(inPath)-4] = '\0';
//             // PUT_3_STRS_TOGETHER(inPath, input_path, "/", entry->d_name);
//             PUT_3_STRS_TOGETHER(outPath, output_path, "/", entry->d_name);
//             outPath[strlen(outPath)-4] = '\0';
            
//             if(access(outPath, 0) < 0)
//                 mkdir(outPath, 0755);
//             #ifdef  CHECK_DECOMPRESS
//             PUT_3_STRS_TOGETHER(oriPath,reference_path,"/",entry->d_name);
//             oriPath[strlen(oriPath)-4] = '\0';
//             #endif
//             result  =  idedup_decompress(inPath, outPath
//             // rawSize  +=  idedup_decompress(inPath, outPath
//             , idpPath
//                 #ifdef  CHECK_DECOMPRESS
//                     // , oriPath
//                     , reference_path
//                 #endif
//             );
//             //
//             rawSize += result[0];
//             counter += result[1];
//             free(result);

//             time = g_timer_elapsed(timer, NULL);
//             printf("%f GB\n", (rawSize-undecodeSize)/1024.0/1024/1024);
//             printf("bandwidth           :  %f MB/s\n", (rawSize-undecodeSize)/time/1024/1024);
//         }
//         closedir(dir);
//     }
//     else 
//     {
//         printf("missing necessary arguments (\"-c\" or \"-d\")\n");
//         exit(EXIT_FAILURE);
//     }

//     time = g_timer_elapsed(timer,NULL);
//     g_timer_destroy(timer);
//     #ifdef CHECK_BUFFER
//     pthread_mutex_destroy(&request.mutex);
//     pthread_mutex_destroy(&miss.mutex);
//     #endif
//     #ifdef COLLISION_RATE
//     pthread_mutex_destroy(&query.mutex);
//     pthread_mutex_destroy(&collision.mutex);
//     #endif
//     #ifdef DEBUG_2
//     pthread_mutex_destroy(&sim_counter_mutex);
//     #endif
//     #ifdef PART_TIME
//     pthread_mutex_destroy(&read_time_mutex);
//     pthread_mutex_destroy(&decode_time_mutex);
//     pthread_mutex_destroy(&detect_time_mutex);
//     pthread_mutex_destroy(&index_time_mutex);
//     pthread_mutex_destroy(&dedup_time_mutex);
//     pthread_mutex_destroy(&rejpeg_time_mutex);
//     pthread_mutex_destroy(&write_time_mutex);
//     pthread_mutex_destroy(&encode_time_mutex);
//     #endif


//     FILE    *fp = fopen("result6.txt","a");
//     fprintf(fp,"%s, %s\n",argv[0],input_path);
//     if(mode == COMPRESS)
//     {
//         fprintf(fp,"thrd_num: %d, feature_num: %d, feature_method: ", MIDDLE_THREAD_NUM, SF_NUM);
//         switch (FEATURE_METHOD)
//         {
//             case _RABIN:
//                 fprintf(fp,"rabin\n");
//                 break;
//             case _GEAR:
//                 fprintf(fp,"gear\n");
//                 break;
//             case _2DF:
//                 fprintf(fp,"feature map\n");
//                 break;
//             default:
//                 break;
//         }
//         fprintf(fp,"block_size: %d, dimension: %d, delta: ", _block_size, ((one_dimension==0)? 2:(one_dimension==1)? 1:3));
//         switch (delta_method)
//         {
//             case XDELTA:
//                 fprintf(fp,"xdelta\n");
//                 break;
//             case IDELTA:
//                 fprintf(fp,"idelta\n");
//                 break;
//             default:
//                 break;
//         }
//         fprintf(fp,"----------------------start------------------------\n\n");
//         fprintf(fp,"compression ratio   :  %f : 1\n",(double)(rawSize-undecodeSize)/finalSize);
//         fprintf(fp,"bandwidth           :  %f MB/s\n", (rawSize-undecodeSize)/time/1024/1024);
//         fprintf(fp,"avaliable size      :  %f GB\n", (rawSize-undecodeSize)/1024.0/1024/1024);
//         fprintf(fp,"compressed size     :  %f GB\n", finalSize/1024.0/1024/1024);
//         fprintf(fp,"unavaliable size    :  %f GB\n", undecodeSize/1024.0/1024/1024);
//         fprintf(fp,"similar size        :  %f GB\n", (rawSize-undecodeSize-sizes[8])/1024.0/1024/1024);
//         fprintf(fp,"similar ratio       :  %f : 1\n",(((rawSize-undecodeSize-sizes[8])/1024.0/1024/1024)/((finalSize-sizes[8])/1024.0/1024/1024)));
//         fprintf(fp,"time                :  %f s\n", time);
//         fprintf(fp,"success rate        :  %.1f%%\n", (1-((float)sizes[8])/(rawSize-undecodeSize))*100);
//         fprintf(fp,"jpeg ratio          :  %f : 1\n", decodedSize/1.0/(rawSize-undecodeSize));
//         fprintf(fp,"detected size ratio :  %f : 1\n", decodedSize/1.0/detectSize);
//         fprintf(fp,"deltaed size ratio  :  %f : 1\n", decodedSize/1.0/deltaSize_decd);
//         fprintf(fp,"omited delta ratio  :  %f\n", omitSize/1.0/deltaSize_raw);
//         #ifdef DEBUG_1
//         fprintf(fp,"\n");
//         for(int i=0; i<8; i++)
//             fprintf(fp,"%.2f%%, %.2f MB\n", (float)sizes[i]/sizes[7]*100, (float)sizes[i]/1024/1024);
//         #endif
//         #ifdef PART_TIME
//         fprintf(fp,"\n");
//         fprintf(fp,"read    : %f MB/s\n", (rawSize-undecodeSize)/1024.0/1024/read_time);
//         fprintf(fp,"decode  : %f MB/s\n", (rawSize-undecodeSize)/1024.0/1024/decode_time);
//         fprintf(fp,"detect  : %f MB/s\n", detectSize/1024.0/1024/detect_time);
//         fprintf(fp,"index   : %f MB/s\n", deltaSize_decd/1024.0/1024/index_time);
//         fprintf(fp,"dedup   : %f MB/s\n", deltaSize_decd/1024.0/1024/dedup_time);
//         fprintf(fp,"delta   : %f MB/s\n", deltaSize_decd/1024.0/1024/(dedup_time+index_time));
//         fprintf(fp,"rejpeg  : %f MB/s\n", deltaSize_raw/1024.0/1024/rejpeg_time);
//         fprintf(fp,"write   : %f MB/s\n", sizes[7]/1024.0/1024/write_time);
//         fprintf(fp,"detect(raw)  : %f MB/s\n", (rawSize-undecodeSize)/1024.0/1024/detect_time);
//         fprintf(fp,"index(raw)   : %f MB/s\n", deltaSize_raw/1024.0/1024/index_time);
//         fprintf(fp,"dedup(raw)   : %f MB/s\n", deltaSize_raw/1024.0/1024/dedup_time);
//         fprintf(fp,"delta(raw)   : %f MB/s\n", deltaSize_raw/1024.0/1024/(dedup_time+index_time));
//         fprintf(fp,"I/O     : %f MB/s\n", (rawSize-undecodeSize)/1024.0/1024/(read_time+write_time));
//         #endif
//         #ifdef CHECK_BUFFER
//         fprintf(fp,"total request       :  %lu\n", request.counter);
//         fprintf(fp,"missing request     :  %lu\n", miss.counter);
//         fprintf(fp,"hit ratio           :  %.1f%%\n", (1-(double)miss.counter/request.counter)*100);    
//         #endif
//         #ifdef COLLISION_RATE
//         fprintf(fp,"collision times     :  %lu\n", query.counter);
//         fprintf(fp,"query times         :  %lu\n", collision.counter);
//         fprintf(fp,"collision rate      :  %.1f%%\n", ((double)collision.counter/query.counter)*100);
//         #endif
//         #ifdef DEBUG_2
//         for(int i=0; i<20; i++)
//             fprintf(fp,"%lu  ", sim_counter[i]);
//         fprintf(fp,"\n");
//         #endif
//         #ifdef PART_TIME
//         fprintf(fp,"read time:  %f mins\n", read_time/60/READ_THREAD_NUM);
//         fprintf(fp,"decode time:%f mins\n", decode_time/60/DECODE_THREAD_NUM);
//         fprintf(fp,"detect time:%f mins\n", detect_time/60/MIDDLE_THREAD_NUM);
//         fprintf(fp,"index time: %f mins\n", index_time/60/MIDDLE_THREAD_NUM);
//         fprintf(fp,"delta time: %f mins\n", dedup_time/60/MIDDLE_THREAD_NUM);
//         fprintf(fp,"rejpeg time:%f mins\n", rejpeg_time/60/MIDDLE_THREAD_NUM);
//         fprintf(fp,"write time: %f mins\n", write_time/60/WRITE_THREAD_NUM);
//         #endif
//     }
//     else
//     {
//         fprintf(fp,"----------------------start------------------------\n\n");
//         fprintf(fp,"thrd_num:   %d\n", MIDDLE_THREAD_NUM);
//         fprintf(fp,"bandwidth           :  %f MB/s\n", rawSize/time/1024/1024);
//         fprintf(fp,"avaliable size      :  %f GB\n", rawSize/1024.0/1024/1024);
//         fprintf(fp,"frequency:          :  %f\n", counter/time);
//         fprintf(fp,"counter:            :  %ld\n", counter);
//         fprintf(fp,"time                :  %f s\n", time);
//         #ifdef PART_TIME
//         fprintf(fp,"\n");
//         fprintf(fp,"read    : %f MB/s\n", rawSize/1024.0/1024/read_time);
//         fprintf(fp,"middle  : %f MB/s\n", rawSize/1024.0/1024/decode_time);
//         fprintf(fp,"decode  : %f MB/s\n", rawSize/1024.0/1024/rejpeg_time);
//         fprintf(fp,"dedup   : %f MB/s\n", rawSize/1024.0/1024/dedup_time);
//         fprintf(fp,"encode  : %f MB/s\n", rawSize/1024.0/1024/encode_time);
//         fprintf(fp,"write   : %f MB/s\n", rawSize/1024.0/1024/write_time);
//         #endif
//     }
//     fprintf(fp,"\n-----------------------end-------------------------\n\n\n");
//     fclose(fp);
// }



/*
 * @Author: Cai Deng
 * @Date: 2020-10-12 08:11:45
 * @LastEditors: Cai Deng
 * @LastEditTime: 2022-05-15 13:05:18
 * @Description: 
 */
#include "idedup.h"
#include <getopt.h>
#include "rabin.h"

#define COMPRESS 1
#define DECOMPRESS 2

int READ_THREAD_NUM;
int DECODE_THREAD_NUM;
int MIDDLE_THREAD_NUM;
int REJPEG_THREAD_NUM;
int WRITE_THREAD_NUM;

int ROAD_NUM;

int64_t DECODE_BUFFER_SIZE;
int64_t PATCH_SIZE;

int64_t NAME_LIST_MAX;
int64_t READ_LIST_MAX;
int64_t DECD_LIST_MAX;
int64_t DECT_LIST_MAX;
int64_t INDX_LIST_MAX;
int64_t DEUP_LIST_MAX;
int64_t REJG_LIST_MAX;

uint8_t chunking_mode;
uint8_t in_chaos;

int SF_NUM, FEA_PER_SF;
uint8_t FEATURE_METHOD;
int _block_size;
uint8_t one_dimension;

#define IDELTA 0
#define XDELTA 1
uint8_t delta_method;
uint8_t data_type;

#ifdef PART_TIME
double read_time = 0;
double decode_time = 0;
double detect_time = 0;
double index_time  = 0;
double dedup_time = 0;
double rejpeg_time = 0;
double write_time = 0;
double encode_time = 0;
pthread_mutex_t detect_time_mutex, index_time_mutex, dedup_time_mutex, read_time_mutex, write_time_mutex, decode_time_mutex, rejpeg_time_mutex, encode_time_mutex;
#endif

#ifdef CHECK_BUFFER
struct buffer_counter {
    uint64_t    counter;
    pthread_mutex_t mutex;
}   request, miss;
#endif
#ifdef COLLISION_RATE
struct collision_counter {
    uint64_t    counter;
    pthread_mutex_t mutex;
}   query, collision;
#endif

#ifdef DEBUG_2
uint64_t sim_counter[20] = {0};
pthread_mutex_t sim_counter_mutex;
#endif

uint64_t out_table_f[256], out_table_i[256], mod_table_f[256], mod_table_i[256];

#define CHECK_POSTFIX(fileName,lastName)  (!strcmp(lastName,(fileName) + strlen((fileName)) - strlen((lastName))))

void main(int argc, char *argv[])
{
    GTimer      *timer = g_timer_new();
    double      time;
    struct      dirent  *entry;
    DIR         *dir;
    char        inPath[MAX_PATH_LEN], outPath[MAX_PATH_LEN], idpPath[MAX_PATH_LEN];
    #ifdef  CHECK_DECOMPRESS
    char        oriPath[MAX_PATH_LEN];
    #endif
    uint64_t    *result, counter = 0, rawSize = 0, undecodeSize = 0, finalSize = 0, decodedSize = 0, deltaSize_raw = 0, deltaSize_decd = 0, detectSize = 0, omitSize = 0;
    #ifdef DEBUG_1
    uint64_t    sizes[9] = {0};
    #endif
    int         option, mode = 0;
    char        *input_path, *output_path, *reference_path;
    static      struct  option  longOptions[]   =   
    {
        {"read_thrd_num", required_argument, NULL, 'a'},
        {"decode_thrd_num", required_argument, NULL, 'B'},
        {"middle_thrd_num", required_argument, NULL, 'b'},
        {"write_thrd_num", required_argument, NULL, 'e'},
        {"input_path", required_argument, NULL, 'f'},
        {"output_path", required_argument, NULL, 'g'},
        {"reference_path", required_argument, NULL, 'h'},
        {"buffer_size", required_argument, NULL, 'i'},
        {"patch_size", required_argument, NULL, 'j'},
        {"name_list", required_argument, NULL, 'k'},
        {"read_list", required_argument, NULL, 'l'},
        {"decd_list", required_argument, NULL, 'm'},
        {"dect_list", required_argument, NULL, 'n'},
        {"indx_list", required_argument, NULL, 'A'},
        {"deup_list", required_argument, NULL, 'o'},
        {"rejg_list", required_argument, NULL, 'p'},
        {"chunking", required_argument, NULL, 'q'},
        {"road_num", required_argument, NULL, 'r'},
        {"chaos", required_argument, NULL, 's'},
        {"sf_num", required_argument, NULL, 't'},
        {"sf_component_num", required_argument, NULL, 'u'},
        {"feature_method", required_argument, NULL, 'v'},
        {"block_size", required_argument, NULL, 'w'},
        {"dimension", required_argument, NULL, 'x'},
        {"delta", required_argument, NULL, 'y'},
        {"data_type", required_argument, NULL, 'z'},
        {"jpeg_list", required_argument, NULL, 'C'},
        {"encd_list", required_argument, NULL, 'D'},
        {"rejpeg_thrd_num", required_argument, NULL, 'E'}
    };

    while((option = getopt_long_only(argc, argv, "cd", longOptions, NULL))!=-1)
    {
        switch (option)
        {
        case 'c':
            mode = COMPRESS;
            break;
        case 'd':
            mode = DECOMPRESS;
            break;
        case 'a':
            READ_THREAD_NUM = atoi(optarg);
            break;
        case 'B':
            DECODE_THREAD_NUM = atoi(optarg);
            break;
        case 'b':
            MIDDLE_THREAD_NUM = atoi(optarg);
            break;
        case 'e':
            WRITE_THREAD_NUM = atoi(optarg);
            break;
        case 'E':
            REJPEG_THREAD_NUM = atoi(optarg);
            break;
        case 'f':
            input_path = optarg;
            break;
        case 'g':
            output_path = optarg;
            break;
        case 'h':
            reference_path = optarg;
            break;
        case 'i':
            if(*optarg=='G' || *optarg=='g')      DECODE_BUFFER_SIZE = ((int64_t)atoi(optarg+1)) << 30;
            else if(*optarg=='M' || *optarg=='m') DECODE_BUFFER_SIZE = ((int64_t)atoi(optarg+1)) << 20;
            else if(*optarg=='K' || *optarg=='k') DECODE_BUFFER_SIZE = ((int64_t)atoi(optarg+1)) << 10;
            else                                  DECODE_BUFFER_SIZE = (int64_t)atoi(optarg);
            break;
        case 'j':
            if(*optarg=='G' || *optarg=='g')      PATCH_SIZE = ((int64_t)atoi(optarg+1)) << 30;
            else if(*optarg=='M' || *optarg=='m') PATCH_SIZE = ((int64_t)atoi(optarg+1)) << 20;
            else if(*optarg=='K' || *optarg=='k') PATCH_SIZE = ((int64_t)atoi(optarg+1)) << 10;
            else                                  PATCH_SIZE = (int64_t)atoi(optarg);
            break;
        case 'k':
            if(*optarg=='G' || *optarg=='g')      NAME_LIST_MAX = ((int64_t)atoi(optarg+1)) << 30;
            else if(*optarg=='M' || *optarg=='m') NAME_LIST_MAX = ((int64_t)atoi(optarg+1)) << 20;
            else if(*optarg=='K' || *optarg=='k') NAME_LIST_MAX = ((int64_t)atoi(optarg+1)) << 10;
            else                                  NAME_LIST_MAX = (int64_t)atoi(optarg);
            break;
        case 'l':
            if(*optarg=='G' || *optarg=='g')      READ_LIST_MAX = ((int64_t)atoi(optarg+1)) << 30;
            else if(*optarg=='M' || *optarg=='m') READ_LIST_MAX = ((int64_t)atoi(optarg+1)) << 20;
            else if(*optarg=='K' || *optarg=='k') READ_LIST_MAX = ((int64_t)atoi(optarg+1)) << 10;
            else                                  READ_LIST_MAX = (int64_t)atoi(optarg);
            break;
        case 'm':
            if(*optarg=='G' || *optarg=='g')      DECD_LIST_MAX = ((int64_t)atoi(optarg+1)) << 30;
            else if(*optarg=='M' || *optarg=='m') DECD_LIST_MAX = ((int64_t)atoi(optarg+1)) << 20;
            else if(*optarg=='K' || *optarg=='k') DECD_LIST_MAX = ((int64_t)atoi(optarg+1)) << 10;
            else                                  DECD_LIST_MAX = (int64_t)atoi(optarg);
            break;
        case 'n':
            if(*optarg=='G' || *optarg=='g')      DECT_LIST_MAX = ((int64_t)atoi(optarg+1)) << 30;
            else if(*optarg=='M' || *optarg=='m') DECT_LIST_MAX = ((int64_t)atoi(optarg+1)) << 20;
            else if(*optarg=='K' || *optarg=='k') DECT_LIST_MAX = ((int64_t)atoi(optarg+1)) << 10;
            else                                  DECT_LIST_MAX = (int64_t)atoi(optarg);
            break;
        case 'A':
            if(*optarg=='G' || *optarg=='g')      INDX_LIST_MAX = ((int64_t)atoi(optarg+1)) << 30;
            else if(*optarg=='M' || *optarg=='m') INDX_LIST_MAX = ((int64_t)atoi(optarg+1)) << 20;
            else if(*optarg=='K' || *optarg=='k') INDX_LIST_MAX = ((int64_t)atoi(optarg+1)) << 10;
            else                                  INDX_LIST_MAX = (int64_t)atoi(optarg);
            break;
        case 'o':
            if(*optarg=='G' || *optarg=='g')      DEUP_LIST_MAX = ((int64_t)atoi(optarg+1)) << 30;
            else if(*optarg=='M' || *optarg=='m') DEUP_LIST_MAX = ((int64_t)atoi(optarg+1)) << 20;
            else if(*optarg=='K' || *optarg=='k') DEUP_LIST_MAX = ((int64_t)atoi(optarg+1)) << 10;
            else                                  DEUP_LIST_MAX = (int64_t)atoi(optarg);
            break;
        case 'p':
            if(*optarg=='G' || *optarg=='g')      REJG_LIST_MAX = ((int64_t)atoi(optarg+1)) << 30;
            else if(*optarg=='M' || *optarg=='m') REJG_LIST_MAX = ((int64_t)atoi(optarg+1)) << 20;
            else if(*optarg=='K' || *optarg=='k') REJG_LIST_MAX = ((int64_t)atoi(optarg+1)) << 10;
            else                                  REJG_LIST_MAX = (int64_t)atoi(optarg);
            break;
        case 'q':
            if(*optarg=='v' || *optarg=='V')        chunking_mode   =   0;
            else                                    chunking_mode   =   1;
            break;
        case 'r':
            ROAD_NUM    =   atoi(optarg);
            break;
        case 's':
            if(*optarg=='y' || *optarg=='Y')     in_chaos   =   1;
            else                                 in_chaos   =   0;
            break;
        case 't':
            SF_NUM  =   atoi(optarg);
            break;
        case 'u':
            FEA_PER_SF  =   atoi(optarg);
            break;
        case 'v':
            if(!strcmp(optarg, "rabin"))     FEATURE_METHOD = _RABIN;
            else if(!strcmp(optarg, "gear")) FEATURE_METHOD = _GEAR;
            else                             FEATURE_METHOD = _2DF;
            break;
        case 'w':
            _block_size =   atoi(optarg);
            break;
        case 'x':
            if(!strcmp(optarg,"1"))     one_dimension = 1;
            else if(!strcmp(optarg,"2"))one_dimension = 0;
            else                        one_dimension = 2;
            break;
        case 'y':
            if(!strcmp(optarg,"xdelta"))    delta_method = XDELTA;
            else                            delta_method = IDELTA;
            break;
        case 'z':
            if(!strcmp(optarg,"decoded"))   data_type = DECODED;
            else                            data_type = RAW;
            break;
        case 'C':
            if(*optarg=='G' || *optarg=='g')      INDX_LIST_MAX = ((int64_t)atoi(optarg+1)) << 30;
            else if(*optarg=='M' || *optarg=='m') INDX_LIST_MAX = ((int64_t)atoi(optarg+1)) << 20;
            else if(*optarg=='K' || *optarg=='k') INDX_LIST_MAX = ((int64_t)atoi(optarg+1)) << 10;
            else                                  INDX_LIST_MAX = (int64_t)atoi(optarg);
            break;
        case 'D':
            if(*optarg=='G' || *optarg=='g')      REJG_LIST_MAX = ((int64_t)atoi(optarg+1)) << 30;
            else if(*optarg=='M' || *optarg=='m') REJG_LIST_MAX = ((int64_t)atoi(optarg+1)) << 20;
            else if(*optarg=='K' || *optarg=='k') REJG_LIST_MAX = ((int64_t)atoi(optarg+1)) << 10;
            else                                  REJG_LIST_MAX = (int64_t)atoi(optarg);
            break;
        default:
            break;
        }
    }

    uint64_t rabin_win_size;
    if(mode == COMPRESS)
    {
        if(FEATURE_METHOD == _RABIN)
        {
            if(one_dimension == 0)  rabin_win_size = _block_size*_block_size*sizeof(JBLOCK);
            else if(one_dimension == 1) rabin_win_size = _block_size;
            else    rabin_win_size = _block_size*sizeof(JBLOCK);
            calc_tables(rabin_win_size,out_table_f,mod_table_f);
        }
        calc_tables(sizeof(JBLOCK),out_table_i,mod_table_i);
    }

    // if(_block_size == 1 && FEATURE_METHOD == _2DF)    one_dimension   =   1;

    #ifdef CHECK_BUFFER
    request.counter = 0;
    miss.counter = 0;
    pthread_mutex_init(&request.mutex, NULL);
    pthread_mutex_init(&miss.mutex, NULL);
    #endif
    #ifdef COLLISION_RATE
    query.counter = 0;
    collision.counter = 0;
    pthread_mutex_init(&query.mutex, NULL);
    pthread_mutex_init(&collision.mutex, NULL);
    #endif
    #ifdef DEBUG_2
    pthread_mutex_init(&sim_counter_mutex,NULL);
    #endif
    #ifdef PART_TIME
    pthread_mutex_init(&read_time_mutex,NULL);
    pthread_mutex_init(&decode_time_mutex,NULL);
    pthread_mutex_init(&detect_time_mutex,NULL);
    pthread_mutex_init(&index_time_mutex,NULL);
    pthread_mutex_init(&dedup_time_mutex,NULL);
    pthread_mutex_init(&rejpeg_time_mutex,NULL);
    pthread_mutex_init(&write_time_mutex,NULL);
    pthread_mutex_init(&encode_time_mutex,NULL);
    #endif

    g_timer_start(timer);

    if(mode == COMPRESS)
    {
        if(!chunking_mode)  READ_THREAD_NUM =   1;  
        /* only when it equals to 1, this system can chunk in variable mode. */
        result  =   idedup_compress(input_path, output_path);
        rawSize +=  result[0];
        undecodeSize    +=  result[1];
        finalSize   +=  result[2];
        #ifdef DEBUG_1
        for(int i=0; i<9; i++)
            sizes[i] += result[3+i];
        #endif
        decodedSize +=  result[12];
        deltaSize_raw += result[13];
        deltaSize_decd += result[14];
        detectSize += result[15];
        omitSize    +=  result[16];

        free(result);
    }
    else if(mode == DECOMPRESS)
    {
        dir =   opendir(input_path);
        while(entry = readdir(dir))
        {
            if(!strcmp(entry->d_name,".") || !strcmp(entry->d_name,"..") 
            // || CHECK_POSTFIX(entry->d_name, "idp")
            || !CHECK_POSTFIX(entry->d_name, "idp")
            // || !strcmp(entry->d_name,"24.idp")
            // || !strcmp(entry->d_name,"16.idp")
            )
                continue ;

            PUT_3_STRS_TOGETHER(idpPath, input_path, "/", entry->d_name);
            strcpy(inPath, idpPath);
            inPath[strlen(inPath)-4] = '\0';
            // PUT_3_STRS_TOGETHER(inPath, input_path, "/", entry->d_name);
            PUT_3_STRS_TOGETHER(outPath, output_path, "/", entry->d_name);
            outPath[strlen(outPath)-4] = '\0';
            
            if(access(outPath, 0) < 0)
                mkdir(outPath, 0755);
            #ifdef  CHECK_DECOMPRESS
            PUT_3_STRS_TOGETHER(oriPath,reference_path,"/",entry->d_name);
            oriPath[strlen(oriPath)-4] = '\0';
            #endif
            result  =  idedup_decompress(inPath, outPath
            // rawSize  +=  idedup_decompress(inPath, outPath
            , idpPath
                #ifdef  CHECK_DECOMPRESS
                    // , oriPath
                    , reference_path
                #endif
            );
            //
            rawSize += result[0];
            counter += result[1];
            free(result);

            time = g_timer_elapsed(timer, NULL);
            printf("%f GB\n", (rawSize-undecodeSize)/1024.0/1024/1024);
            printf("bandwidth           :  %f MB/s\n", (rawSize-undecodeSize)/time/1024/1024);
        }
        closedir(dir);
    }
    else 
    {
        printf("missing necessary arguments (\"-c\" or \"-d\")\n");
        exit(EXIT_FAILURE);
    }

    time = g_timer_elapsed(timer,NULL);
    g_timer_destroy(timer);
    #ifdef CHECK_BUFFER
    pthread_mutex_destroy(&request.mutex);
    pthread_mutex_destroy(&miss.mutex);
    #endif
    #ifdef COLLISION_RATE
    pthread_mutex_destroy(&query.mutex);
    pthread_mutex_destroy(&collision.mutex);
    #endif
    #ifdef DEBUG_2
    pthread_mutex_destroy(&sim_counter_mutex);
    #endif
    #ifdef PART_TIME
    pthread_mutex_destroy(&read_time_mutex);
    pthread_mutex_destroy(&decode_time_mutex);
    pthread_mutex_destroy(&detect_time_mutex);
    pthread_mutex_destroy(&index_time_mutex);
    pthread_mutex_destroy(&dedup_time_mutex);
    pthread_mutex_destroy(&rejpeg_time_mutex);
    pthread_mutex_destroy(&write_time_mutex);
    pthread_mutex_destroy(&encode_time_mutex);
    #endif


    FILE    *fp = fopen("result6.txt","a");
    fprintf(fp,"%s, %s\n",argv[0],input_path);
    if(mode == COMPRESS)
    {
        fprintf(fp,"thrd_num: %d, feature_num: %d, feature_method: ", MIDDLE_THREAD_NUM, SF_NUM);
        switch (FEATURE_METHOD)
        {
            case _RABIN:
                fprintf(fp,"rabin\n");
                break;
            case _GEAR:
                fprintf(fp,"gear\n");
                break;
            case _2DF:
                fprintf(fp,"feature map\n");
                break;
            default:
                break;
        }
        fprintf(fp,"block_size: %d, dimension: %d, delta: ", _block_size, ((one_dimension==0)? 2:(one_dimension==1)? 1:3));
        switch (delta_method)
        {
            case XDELTA:
                fprintf(fp,"xdelta\n");
                break;
            case IDELTA:
                fprintf(fp,"idelta\n");
                break;
            default:
                break;
        }
        fprintf(fp,"----------------------start------------------------\n\n");
        fprintf(fp,"compression ratio   :  %f : 1\n",(double)(rawSize-undecodeSize)/finalSize);
        fprintf(fp,"bandwidth           :  %f MB/s\n", (rawSize-undecodeSize)/time/1024/1024);
        fprintf(fp,"avaliable size      :  %f GB\n", (rawSize-undecodeSize)/1024.0/1024/1024);
        fprintf(fp,"compressed size     :  %f GB\n", finalSize/1024.0/1024/1024);
        fprintf(fp,"unavaliable size    :  %f GB\n", undecodeSize/1024.0/1024/1024);
        fprintf(fp,"similar size        :  %f GB\n", (rawSize-undecodeSize-sizes[8])/1024.0/1024/1024);
        fprintf(fp,"similar ratio       :  %f : 1\n",(((rawSize-undecodeSize-sizes[8])/1024.0/1024/1024)/((finalSize-sizes[8])/1024.0/1024/1024)));
        fprintf(fp,"time                :  %f s\n", time);
        fprintf(fp,"success rate        :  %.1f%%\n", (1-((float)sizes[8])/(rawSize-undecodeSize))*100);
        fprintf(fp,"jpeg ratio          :  %f : 1\n", decodedSize/1.0/(rawSize-undecodeSize));
        fprintf(fp,"detected size ratio :  %f : 1\n", decodedSize/1.0/detectSize);
        fprintf(fp,"deltaed size ratio  :  %f : 1\n", decodedSize/1.0/deltaSize_decd);
        fprintf(fp,"omited delta ratio  :  %f\n", omitSize/1.0/deltaSize_raw);
        #ifdef DEBUG_1
        fprintf(fp,"\n");
        for(int i=0; i<8; i++)
            fprintf(fp,"%.2f%%, %.2f MB\n", (float)sizes[i]/sizes[7]*100, (float)sizes[i]/1024/1024);
        #endif
        #ifdef PART_TIME
        fprintf(fp,"\n");
        fprintf(fp,"read    : %f MB/s\n", (rawSize-undecodeSize)/1024.0/1024/read_time);
        fprintf(fp,"decode  : %f MB/s\n", (rawSize-undecodeSize)/1024.0/1024/decode_time);
        fprintf(fp,"detect  : %f MB/s\n", detectSize/1024.0/1024/detect_time);
        fprintf(fp,"index   : %f MB/s\n", deltaSize_decd/1024.0/1024/index_time);
        fprintf(fp,"dedup   : %f MB/s\n", deltaSize_decd/1024.0/1024/dedup_time);
        fprintf(fp,"delta   : %f MB/s\n", deltaSize_decd/1024.0/1024/(dedup_time+index_time));
        fprintf(fp,"rejpeg  : %f MB/s\n", deltaSize_raw/1024.0/1024/rejpeg_time);
        fprintf(fp,"write   : %f MB/s\n", sizes[7]/1024.0/1024/write_time);
        fprintf(fp,"detect(raw)  : %f MB/s\n", (rawSize-undecodeSize)/1024.0/1024/detect_time);
        fprintf(fp,"index(raw)   : %f MB/s\n", deltaSize_raw/1024.0/1024/index_time);
        fprintf(fp,"dedup(raw)   : %f MB/s\n", deltaSize_raw/1024.0/1024/dedup_time);
        fprintf(fp,"delta(raw)   : %f MB/s\n", deltaSize_raw/1024.0/1024/(dedup_time+index_time));
        fprintf(fp,"I/O     : %f MB/s\n", (rawSize-undecodeSize)/1024.0/1024/(read_time+write_time));
        #endif
        #ifdef CHECK_BUFFER
        fprintf(fp,"total request       :  %lu\n", request.counter);
        fprintf(fp,"missing request     :  %lu\n", miss.counter);
        fprintf(fp,"hit ratio           :  %.1f%%\n", (1-(double)miss.counter/request.counter)*100);    
        #endif
        #ifdef COLLISION_RATE
        fprintf(fp,"collision times     :  %lu\n", query.counter);
        fprintf(fp,"query times         :  %lu\n", collision.counter);
        fprintf(fp,"collision rate      :  %.1f%%\n", ((double)collision.counter/query.counter)*100);
        #endif
        #ifdef DEBUG_2
        for(int i=0; i<20; i++)
            fprintf(fp,"%lu  ", sim_counter[i]);
        fprintf(fp,"\n");
        #endif
        #ifdef PART_TIME
        fprintf(fp,"read time:  %f mins\n", read_time/60/READ_THREAD_NUM);
        fprintf(fp,"decode time:%f mins\n", decode_time/60/DECODE_THREAD_NUM);
        fprintf(fp,"detect time:%f mins\n", detect_time/60/MIDDLE_THREAD_NUM);
        fprintf(fp,"index time: %f mins\n", index_time/60/MIDDLE_THREAD_NUM);
        fprintf(fp,"delta time: %f mins\n", dedup_time/60/MIDDLE_THREAD_NUM);
        fprintf(fp,"rejpeg time:%f mins\n", rejpeg_time/60/REJPEG_THREAD_NUM);
        fprintf(fp,"write time: %f mins\n", write_time/60/WRITE_THREAD_NUM);
        #endif
    }
    else
    {
        fprintf(fp,"----------------------start------------------------\n\n");
        fprintf(fp,"thrd_num:   %d\n", MIDDLE_THREAD_NUM);
        fprintf(fp,"bandwidth           :  %f MB/s\n", rawSize/time/1024/1024);
        fprintf(fp,"avaliable size      :  %f GB\n", rawSize/1024.0/1024/1024);
        fprintf(fp,"frequency:          :  %f\n", counter/time);
        fprintf(fp,"counter:            :  %ld\n", counter);
        fprintf(fp,"time                :  %f s\n", time);
        #ifdef PART_TIME
        fprintf(fp,"\n");
        fprintf(fp,"read    : %f MB/s\n", rawSize/1024.0/1024/read_time);
        fprintf(fp,"middle  : %f MB/s\n", rawSize/1024.0/1024/decode_time);
        fprintf(fp,"decode  : %f MB/s\n", rawSize/1024.0/1024/rejpeg_time);
        fprintf(fp,"dedup   : %f MB/s\n", rawSize/1024.0/1024/dedup_time);
        fprintf(fp,"encode  : %f MB/s\n", rawSize/1024.0/1024/encode_time);
        fprintf(fp,"write   : %f MB/s\n", rawSize/1024.0/1024/write_time);
        #endif
    }
    fprintf(fp,"\n-----------------------end-------------------------\n\n\n");
    fclose(fp);
}