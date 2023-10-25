# imDedup_plus

## Pipeline Structure

We exploited pipeline to speed up the deduplication process. The pipeline structure is shown in the figure below. There are five pipes which can be divided into three modules.
The first module includes the "decode" pipe, which decodes incoming JPEG images into quantized DCT blocks. The decoded image is the target sent to the next pipe.
The second module includes the "detect" pipe, which detects a similar candidate for the target image. This pipe generates a Feature Bitmap for each decoded image and extracts features from the Bitmap. Then, it selects the image having the most number of identical features with the incoming image as the base. The target and base are sent to the next pipe.
The third module includes the "indexing", "delta", and "recompress" pipes, whcih compresses the target image by Idelta.
The three pipes work together to achieve Idelta's function.
The "indexing" pipe generates an index for each DCT block of base.
The "delta" pipe queries the index to locate all redundant blocks between target and base and represents them with "INSERT" instructions.
The "recompress" pipe finally recompresses those non-redundant DCT blocks by the original JPEG entropy encoding.

![image](https://github.com/DDDCai/imDedup_plus/assets/29863262/5e117758-7691-4906-ba48-a0894ec0b62e)
