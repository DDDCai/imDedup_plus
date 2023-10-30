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

### Compress

${program_name} -c --input_path=${data_path} --output_path=${out_path} --read_thrd_num=1 --decode_thrd_num=10 --middle_thrd_num=1 --rejpeg_thrd_num=10 --write_thrd_num=1 --buffer_size=G64 --patch_size=G1 --name_list=G1 --read_list=G2 --indx_list=G2 --decd_list=G2 --dect_list=G2 --deup_list=G2 --rejg_list=G2 --chunking=variable --road_num=1 --chaos=no --sf_num=10 --sf_component_num=1 --feature_method=2df --block_size=2 --dimension=2 --delta=idelta --data_type=decoded

-c/d:  compress/decompress
