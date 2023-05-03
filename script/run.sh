#! /bin/bash

rm -r /home/dc/data/taobao/dst/
mkdir /home/dc/data/taobao/dst

for file in `ls /home/dc/data/taobao/100-100-100-100-100-100/`
do 
    if [ -d /home/dc/data/taobao/100-100-100-100-100-100/$file ]
    then 
        mkdir /home/dc/data/taobao/dst/$file/
    fi 
done

/home/dc/idedup/build/sid -c /home/dc/data/taobao/100-100-100-100-100-100/ /home/dc/data/taobao/dst/