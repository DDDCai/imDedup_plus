# imDedup_plus

## Pipeline Structure

We exploited pipeline to speed up the deduplication process. The pipeline structure is shown as the figure below. There are five pipes which can be divided into three modules.

![image](https://github.com/DDDCai/imDedup_plus/assets/29863262/79212839-6d9f-4528-a049-bdb71ae632c6)



The first module includes the "decode" pipe, which decodes incoming JPEG images into quantized DCT blocks. The decoded image is sent to the next moudle as the target.

The second module includes the "detect" pipe, which detects a similar candidate for the target image. This pipe generates a Feature Bitmap for each decoded image and extracts features from the Bitmap. Then, it selects the image having the most number of identical features with the target as the base. The target and base are sent to the next moudle.

The third module includes the "indexing", "delta", and "recompress" pipes, whcih compresses the target image by Idelta.
The three pipes work together to achieve Idelta's function.
The "indexing" pipe generates an index for each DCT block of base.
The "delta" pipe queries the index to locate all redundant blocks between target and base and represents them with "INSERT" instructions.
The "recompress" pipe finally recompresses those non-redundant DCT blocks by the original JPEG entropy encoding.
This moudle outputs the compressed target.
