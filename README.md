# imDedup_plus

imDedup_plus is a lossless deduplication method to detect and eliminate fine-grained redundancy between similar JPEG images.  

The paper: https://ieeexplore.ieee.org/document/10423913/

## Pipeline Structure

We exploited pipeline to speed up the deduplication process. The pipeline structure is shown as the figure below. There are five pipes which can be divided into three modules.

![image](https://github.com/DDDCai/imDedup_plus/assets/29863262/b4a1fc3d-2487-425e-a961-e622123517e8)

The first module includes the "decode" pipe, which decodes incoming JPEG images into quantized DCT blocks. The decoded image is sent to the next moudle as the target.

The second module includes the "detect" pipe, which detects a similar candidate for the target image. This pipe generates a Feature Bitmap for each decoded image and extracts features from the Bitmap. Then, it selects the image having the most number of identical features with the target as the base. The target and base are sent to the next moudle.

The third module includes the "indexing", "delta", and "recompress" pipes, whcih compresses the target image by Idelta.
The three pipes work together to achieve Idelta's function.
The "indexing" pipe generates an index for each DCT block of base.
The "delta" pipe queries the index to locate all redundant blocks between target and base and represents them with "INSERT" instructions.
The "recompress" pipe finally recompresses those non-redundant DCT blocks by the original JPEG entropy encoding.
This moudle outputs the compressed target.

## How to run it

### Environment

Ubuntu 18.04

### Requirements

CMake  
Glib-2.0

### Build

cd ${build_folder}  
cmake ${CMakeLists.txt_folder}  
make

### Run

cd ${build_folder}

__*+ COMPRESS*__:  
${program_name} -c --input_path=${data_path} --output_path=${out_path} --read_thrd_num=1 --decode_thrd_num=10 --middle_thrd_num=1 --rejpeg_thrd_num=10 --write_thrd_num=1 --buffer_size=G64 --patch_size=G1 --name_list=G1 --read_list=G2 --indx_list=G2 --decd_list=G2 --dect_list=G2 --deup_list=G2 --rejg_list=G2 --chunking=variable --road_num=1 --sf_num=10 --sf_component_num=1 --feature_method=2df --block_size=2 --dimension=2 --delta=idelta --data_type=decoded

__*+ DECOMPRESS*__:  
${program_name} -d --input_path=${data_path} --output_path=${out_path} --middle_thrd_num=4 --buffer_size=G64 --read_list=G2 --jpeg_list=G2 --decd_list=G2 --deup_list=G2 --encd_list=G2 --reference_path=${ref_path}

### Parameters

__*+ COMPRESS*__:  
__[--input_path]__:      The folder storing dataset. It should contain at least one subfolder. e.g., The dataset is divided into two parts (according to user_name or something), so the ${input_path} contains two subfolders storing the two parts of data, respectively.  
__[--output_path]__:     The folder for storing compressed images and non-redundant images.  
__[--read/decode/rejpeg/write_thrd_num]__:  The number of threads allocated for the corresponding pipe. (read and write pipe are not included in the above figure; they are used to read raw images and write compressed images)  
__[--middle_thrd_num]__: The number of threads allocated for the other pipes excluding pipes listed individually.  
__[--buffer_size]__:     The size of buffer used to store decompressed images. A buffered image can help to reduce the time of reading and decoding when it is selected as base. (e.g., G2 means 2GB, and M200 means 200MB)  
__[--patch_size]__:      imDedup_plus caches the compressed images before it writes them to the storage. The maximum size of cached images is ${patch_size}.
__[--xx_list]__:         The allocated size of each list transferring intermediate results between two pipes.  
__[--chunking]__:        variable/fixed. (If variable, only when the current subfolder has been completely processed will it close a write batch, even though it has reached the patch_size)  
__[--road_num]__:        The number of pipelines runing concurrently.  
__[--sf_num]__:          The number of super feature.  
__[--sf_component_num]__:The number of features each super feature contains.  
__[--feature_method]__:  2df(Feature Bitmap)/rabin/gear
__[--block_size]__:      The size of sliding window walking through the Feature Bitmap. (e.g., 2 means the window size is 2x2 blocks)  
__[--dimension]__:       1(treated as 1-d byte stream like how traditional deduplication does)/2(2-d image block structure).  
__[--delta]__:           idelta/xdelta.  
__[--data_type]__:       decoded/raw.  

__*+ DECOMPRESS*__:  
__[--input_path]__:      The folder storing compressed images (i.e., the *output_path* of *COMPRESS* mode).  
__[--output_path]__:     The folder for storing restored images.  
__[--reference_path]__:  In case that you want to check if the restored images are identical with the original ones, put *all* original images in other folders into the one single ref-folder so that the program can locate and compare them.  

*an example: script/run.sh*  

## Data

The python script we used to produce the simulated dataset is provided in /script/wm.py.

We also provide an instance dataset: https://pan.baidu.com/s/1qREoNOV1cvwk8nw6Pcaoag?pwd=b0xx

## Switches

*in "idedup.h"*  


__[CHECK_DECOMPRESS]__:  turn on to check if the images are correctly restored by the decompression. ("--reference_path" is needed)  
__[HEADER_DELTA]__:      turn on to use xdelta to compress the JPEG header.  
__[JPEG_SEPA_COMP]__:    turn on to compress the Y, U, and V data seperately. (NOTICE: WE DID NOT IMPLEMENT ITS DECOMPRESSION)  
__[DC_HASH]__:           turn on to replace Adler32 with DCHash in Idelta.  
__[FIX_OPTI]__:          turn on to dynamically exploit Fixed-Point-Matching.  
__[IMDEDUP_PLUS]__:      turn on to use imDedup_plus which turns on DC_HASH and FIX_OPTI by default.  
__[ORIGINAL_HUFF]__:     turn on to use the original Huffman table of the processing image to compress the non-redundant blocks. (NOTICE: WE DID NOT IMPLEMENT ITS DECOMPRESSION)  
