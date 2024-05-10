#! /bin/bash

sudo sync
sudo echo 3 > /proc/sys/vm/drop_caches
sudo rm -r /home/dc/data/image_data/dst/test/*
sudo /home/dc/image_dedup/imDedup/build/sid -c --input_path=/home/dc/data/image_data/test/ \
        --output_path=/home/dc/data/image_data/dst/test/ --read_thrd_num=1 --decode_thrd_num=8 --middle_thrd_num=4 --rejpeg_thrd_num=8 \
        --write_thrd_num=1 --buffer_size=G64 --patch_size=G1 --name_list=G1 --read_list=G2 --indx_list=G2 \
        --decd_list=G2 --dect_list=G2 --deup_list=G2 --rejg_list=G2 --chunking=variable \
        --road_num=1 --chaos=no --sf_num=8 --sf_component_num=1 --feature_method=2df \
        --block_size=2 --dimension=2 --delta=idelta --data_type=decoded




sudo sync
sudo echo 3 > /proc/sys/vm/drop_caches
sudo rm -r /home/dc/data/image_data/rst/test/*
sudo /home/dc/image_dedup/imDedup/build/sid -d --input_path=/home/dc/data/image_data/dst/test/ \
        --output_path=/home/dc/data/image_data/rst/test/ --middle_thrd_num=4 --buffer_size=G64 --read_list=G2 \
        --jpeg_list=G2 --decd_list=G2 --deup_list=G2 --encd_list=G2 \
        --reference_path=/home/dc/data/image_data/test_all/
