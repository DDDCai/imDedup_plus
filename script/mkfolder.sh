###
 # @Author: Cai Deng
 # @Date: 2020-07-10 04:13:31
 # @LastEditors: Cai Deng
 # @LastEditTime: 2020-07-21 12:58:42
 # @Description: 
### 

for file in `ls $1`
do 
    if [ -d $1"/"$file ]
    then 
        mkdir $2/$file
    fi 
done

# for file in `ls /data/taobao/taobao/`
# do 
#     if [ -d "/data/taobao/taobao/"$file ]
#     then 
#         sudo mv /data/taobao/taobao/$file /data/taobao/
#     fi
# done