# imDedup_plus

imDedup_plus is a lossless deduplication method to detect and eliminate fine-grained redundancy between similar JPEG images.

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

### Requirements

CMake  
Glib-2.0

### Build it

cd ${build_folder}  
cmake ${CMakeLists.txt_folder}  
make

### Run

cd ${build_folder}

${program_name} -c --input_path=${data_path} --output_path=${out_path} --read_thrd_num=1 --decode_thrd_num=10 --middle_thrd_num=1 --rejpeg_thrd_num=10 --write_thrd_num=1 --buffer_size=G64 --patch_size=G1 --name_list=G1 --read_list=G2 --indx_list=G2 --decd_list=G2 --dect_list=G2 --deup_list=G2 --rejg_list=G2 --chunking=variable --road_num=1 --sf_num=10 --sf_component_num=1 --feature_method=2df --block_size=2 --dimension=2 --delta=idelta --data_type=decoded

### Parameters

__--input_path__:  The folder storing dataset. It should contain at least one subfolder. e.g., The dataset is divided into two parts (according to user_name or something), so the ${input_path} contains two subfolders storing the two parts of data, respectively.  
__--output_path__:  The folder storing compressed images and non-redundant images.  
__--read/decode/rejpeg/write_thrd_num__:  The number of threads allocated for the corresponding pipe. (read and write pipe are not included in the above figure; they are used to read raw images and write compressed images)  
__--middle_thrd_num__:  The number of threads allocated for the other pipes excluding pipes listed individually.  
__--buffer_size__:  The size of buffer used to store decompressed images. A buffered image can help to reduce the time of reading and decoding when it is selected as base. (e.g., G2 means 2GB, and M200 means 200MB)  
__--patch_size__:  imDedup_plus caches the compressed images before it reads and processes the ${patch_size} of images.   
__--xx_list__:  The allocated size of each list transferring intermediate results between two pipes.  
__--chunking__:  variable/fixed. (If variable, only when the current subfolder has been completely processed will it close a write batch, even though it has reached the patch_size)  
__--road_num__:  The number of pipelines runing concurrently.  
__--sf_num__:  The number of super feature.  
__--sf_component_num__:  The number of features each super feature contains.  
__--feature_method__:  2df(Feature Bitmap)/rabin/gear
__--block_size__:  The size of sliding window walking through the Feature Bitmap. (e.g., 2 means the window size is 2x2 blocks)  
__--dimension__:  1(trated as 1-d byte stream like traditional deduplication does)/2(2-d image block structure).  
__--delta__:  idelta/xdelta.  
__--data_type__:  decoded/raw.  

## Data

The python script we used to produce the simulated dataset is provided in /script/wm.py.
