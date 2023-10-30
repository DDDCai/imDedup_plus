'''
Author: Cai Deng
Date: 2021-03-30 08:09:13
LastEditors: Cai Deng
LastEditTime: 2022-10-08 02:34:56
Description: 
'''

import os,traceback
from PIL import Image
import random
 
#############################################################################################################################


# def get_folder(fpath,save_path):
#     try:
#         img_suffix_list = ['png', 'jpg', 'bmp']
#         for i in os.listdir(fpath):
#             if i.split('.')[-1] in img_suffix_list:
#                 # wmID = random.sample(["/home/dc/data/image_data/1650.png","/home/dc/data/image_data/big.png","/home/dc/data/image_data/heart.png", \
#                 #     "/home/dc/data/image_data/idedup.png","/home/dc/data/image_data/sun.png","/home/dc/data/image_data/moon.png"], k=4)
#                 # wmID = ["/home/dc/data/image_data/1650.png","/home/dc/data/image_data/big.png","/home/dc/data/image_data/heart.png", \
#                 #     "/home/dc/data/image_data/idedup.png","/home/dc/data/image_data/sun.png","/home/dc/data/image_data/moon.png"]
#                 wmID = ["/home/dc/data/image_data/self-made/mark1.png","/home/dc/data/image_data/self-made/mark2.png","/home/dc/data/image_data/self-made/mark3.png", \
#                     "/home/dc/data/image_data/self-made/mark4.png","/home/dc/data/image_data/self-made/mark5.png","/home/dc/data/image_data/self-made/mark6.png"]
#                 img_path = fpath + '/' + i
#                 for j in [0,1,2,3,4,5]:
#                     img_water_mark(img_file=img_path,wm_file=wmID[j],save_path=save_path,order=j)
#     except Exception as e:
#         print(traceback.print_exc())
 

# def img_water_mark(img_file, wm_file,save_path,order):
#     try:
#         img = Image.open(img_file)  
#         watermark = Image.open(wm_file)  
#         img_size = img.size
#         wm_size = watermark.size
        
#         # while img_size[0] < wm_size[0]:
#         #     watermark.resize(tuple(map(lambda x: int(x * 0.5), watermark.size)))
#         #     wm_size = watermark.size
#         # print('image size: ', img_size)
#         w = img_size[0]-wm_size[0]
#         h = img_size[1]-wm_size[1]
#         if w<=0:
#             w = 1
#         if h<=0 :
#             h = 1
#         wm_position = (random.randrange(0,w,1),random.randrange(0,h,1))

#         # wm_position = (img_size[0]-wm_size[0],img_size[1]-wm_size[1])
#         layer = Image.new('RGBA', img.size)  
#         layer.paste(watermark, wm_position)  
#         mark_img = Image.composite(layer, img, layer)
#         new_file_name = str(order)+'_'+img_file.split('/')[-1]
#         mark_img.save(save_path + new_file_name,quality=95,subsampling=0)
#     except Exception as e:
#         print(traceback.print_exc())

# get_folder("/home/dc/data/image_data/VOC2012PARTS/0/","/home/dc/data/image_data/WMVOC2012/0/")
# get_folder("/home/dc/data/image_data/VOC2012PARTS/1/","/home/dc/data/image_data/WMVOC2012/1/")
# get_folder("/home/dc/data/image_data/VOC2012PARTS/2/","/home/dc/data/image_data/WMVOC2012/2/")
# get_folder("/home/dc/data/image_data/VOC2012PARTS/3/","/home/dc/data/image_data/WMVOC2012/3/")
# get_folder("/home/dc/data/image_data/VOC2012PARTS/4/","/home/dc/data/image_data/WMVOC2012/4/")
# get_folder("/home/dc/data/image_data/VOC2012PARTS/5/","/home/dc/data/image_data/WMVOC2012/5/")
# get_folder("/home/dc/data/image_data/VOC2012PARTS/6/","/home/dc/data/image_data/WMVOC2012/6/")
# get_folder("/home/dc/data/image_data/VOC2012PARTS/7/","/home/dc/data/image_data/WMVOC2012/7/")
# get_folder("/home/dc/data/image_data/VOC2012PARTS/8/","/home/dc/data/image_data/WMVOC2012/8/")
# get_folder("/home/dc/data/image_data/VOC2012PARTS/9/","/home/dc/data/image_data/WMVOC2012/9/")


############################################################################################################################


# wmID = ["/home/dc/data/image_data/self-made/600X200/mark_1.jpg","/home/dc/data/image_data/self-made/600X200/mark_2.jpg","/home/dc/data/image_data/self-made/600X200/mark_3.jpg", \
#                     "/home/dc/data/image_data/self-made/600X200/mark_4.jpg","/home/dc/data/image_data/self-made/600X200/mark_5.jpg","/home/dc/data/image_data/self-made/600X200/mark_6.jpg"]

# def get_folder(fpath,save_path):
#     try:
#         img_suffix_list = ['png', 'jpg', 'bmp']
#         for i in os.listdir(fpath):
#             if i.split('.')[-1] in img_suffix_list:
#                 img_path = fpath + '/' + i
#                 j = random.randrange(0,5,1)
#                 img_water_mark(img_file=img_path,wm_file=wmID[j],save_path=save_path)
#     except Exception as e:
#         print(traceback.print_exc())
 

# def img_water_mark(img_file, wm_file,save_path):
#     try:
#         img = Image.open(img_file)  
#         watermark = Image.open(wm_file)  
#         img_size = img.size
#         wm_size = watermark.size
        
#         i = random.randrange(0,1,1)
#         w = 0
#         h = 0
#         if i <= 0:
#             h = img_size[1] - wm_size[1]
#             if h <= 0 :
#                 h = 0
        
#         wm_position = (w,h)

#         # wm_position = (img_size[0]-wm_size[0],img_size[1]-wm_size[1])
#         layer = Image.new('RGBA', img.size)  
#         layer.paste(watermark, wm_position)  
#         mark_img = Image.composite(layer, img, layer)
#         new_file_name = 'new_'+img_file.split('/')[-1]
#         mark_img.save(save_path + new_file_name,quality=95,subsampling=0)
#     except Exception as e:
#         print(traceback.print_exc())

# get_folder("/home/dc/data/image_data/VOC2012/JPEGImages/","/home/dc/data/image_data/WMVOC2012_600X200/0/")


####################################################################################################################


def get_folder(fpath,save_path,times):
    try:
        number = 0
        img_suffix_list = ['png', 'jpg', 'bmp', 'JPEG']
        for i in os.listdir(fpath):
            if i.split('.')[-1] in img_suffix_list:
                img_path = fpath + '/' + i
                if number<10000:
                    for j in [0,1]:
                        # wm_path = random.sample(["/home/dc/data/image_data/self-made/mark/13.png","/home/dc/data/image_data/self-made/mark/1.png","/home/dc/data/image_data/self-made/mark/2.png", \
                        # "/home/dc/data/image_data/self-made/mark/3.png","/home/dc/data/image_data/self-made/mark/4.png","/home/dc/data/image_data/self-made/mark/5.png", \
                        #     "/home/dc/data/image_data/self-made/mark/6.png","/home/dc/data/image_data/self-made/mark/7.png","/home/dc/data/image_data/self-made/mark/8.png", \
                        #         "/home/dc/data/image_data/self-made/mark/9.png","/home/dc/data/image_data/self-made/mark/10.png", \
                        #             "/home/dc/data/image_data/self-made/mark/11.png","/home/dc/data/image_data/self-made/mark/12.png", \
                        #                 "/home/dc/data/image_data/self-made/mark/14.png","/home/dc/data/image_data/self-made/mark/15.png", \
                        #                     "/home/dc/data/image_data/self-made/mark/16.png","/home/dc/data/image_data/self-made/mark/17.png", \
                        #                         "/home/dc/data/image_data/self-made/mark/18.png"],k=times)
                        # wm_path = random.sample(["/home/dc/data/image_data/self-made/holmes/holmes_1.jpg",
                        #                          "/home/dc/data/image_data/self-made/holmes/holmes_2.jpg",
                        #                          "/home/dc/data/image_data/self-made/holmes/holmes_3.jpg",
                        #                          "/home/dc/data/image_data/self-made/holmes/holmes_4.jpg",
                        #                          "/home/dc/data/image_data/self-made/holmes/holmes_5.jpg",
                        #                          "/home/dc/data/image_data/self-made/holmes/holmes_6.jpg"],k=times)
                        wm_path = random.sample(os.listdir("/home/dc/data/image_data/logo/"),k=times)
                        img_water_mark(img_file=img_path,wm_file=wm_path,save_path=save_path,order=j,time=times)
                    number = number + 1
                else :
                    img_blank_mark(img_file=img_path,wm_file=wm_path,save_path=save_path)
    except Exception as e:
        print(traceback.print_exc())
 

def img_water_mark(img_file, wm_file,save_path,order,time):
    try:
        img = Image.open(img_file) 
        img_size = img.size 
        watermark = []
        wm_size = []
        w = []
        h = []
        wm_position = []
        for num in range(0,time,1):
            raw_wm=Image.open("/home/dc/data/image_data/logo/" + wm_file[num])
            watermark.append(raw_wm.resize((80,80)))  
            wm_size.append(watermark[num].size)
            w.append(img_size[0]-wm_size[num][0])
            h.append(img_size[1]-wm_size[num][1])
            if w[num]<=0:
                w[num] = 1
            if h[num]<=0 :
                h[num] = 1
            flag=0
            counter=0
            position_tmp = (random.randrange(0,w[num],1),random.randrange(0,h[num],1))
            while flag<=0:
                if counter>=100:
                    break
                for k in range(0,num,1):
                    if position_tmp[0] >= (wm_position[k][0]-wm_size[num][0]):
                        if position_tmp[0] <= (wm_position[k][0]+wm_size[k][0]):
                            if position_tmp[1] >= (wm_position[k][1]-wm_size[num][1]):
                                if position_tmp[1] <= (wm_position[k][1]+wm_size[k][1]):
                                    flag=flag-1
                                    position_tmp = (random.randrange(0,w[num],1),random.randrange(0,h[num],1))
                                    counter=counter+1
                                    break
                flag=flag+1
            wm_position.append(position_tmp)

        # wm_position = (img_size[0]-wm_size[0],img_size[1]-wm_size[1])
        layer = Image.new('RGBA', img.size)  
        for num in range(0,time,1):
            layer.paste(watermark[num], wm_position[num])  
        mark_img = Image.composite(layer, img, layer)
        new_file_name = str(order)+'_'+img_file.split('/')[-1]
        
        mark_img.save(save_path + new_file_name,quality=75)
    except Exception as e:
        print(traceback.print_exc())

def img_blank_mark(img_file, wm_file,save_path):
    try:
        img = Image.open(img_file) 
        img_size = img.size
        new_file_name = img_file.split('/')[-1]
        img.save(save_path + new_file_name,quality=75)
    except Exception as e:
        print(traceback.print_exc())



# def img_water_mark(img_file, wm_file,save_path,order,time):
#     try:
#         img = Image.open(img_file) 
#         img_size = img.size 
#         w=4
#         h=5
#         if img_size[0]>=img_size[1]:
#             w=5
#             h=4
#         watermark = []
#         wm_size = []
#         wm_position = []
#         for num in range(0,time,1):
#             watermark.append(Image.open(wm_file[num]))  
#             wm_size.append(watermark[num].size)
#             flag=0
#             counter=0
#             position_tmp = (random.randrange(0,w,1),random.randrange(0,h,1))
#             while flag<=0:
#                 if counter>=100:
#                     break
#                 for k in range(0,num,1):
#                     if position_tmp[0] == (wm_position[k][0]):
#                         if position_tmp[1] == (wm_position[k][1]):
#                             flag=flag-1
#                             position_tmp = (random.randrange(0,w,1),random.randrange(0,h,1))
#                             counter=counter+1
#                             break
#                 flag=flag+1
#             wm_position.append(position_tmp)

#         # wm_position = (img_size[0]-wm_size[0],img_size[1]-wm_size[1])
#         layer = Image.new('RGBA', img.size)  
#         for num in range(0,time,1):
#             tmp = (wm_position[num][0]*102,wm_position[num][1]*98)
#             layer.paste(watermark[num], tmp)  
#         mark_img = Image.composite(layer, img, layer)
#         new_file_name = str(order)+'_'+img_file.split('/')[-1]
        
#         mark_img.save(save_path + new_file_name,quality=95,subsampling=0)
#     except Exception as e:
#         print(traceback.print_exc())


# def img_water_mark(img_file, wm_file,save_path,order,time):
#     try:
#         img = Image.open(img_file)  
#         watermark1 = Image.open("/home/dc/data/image_data/micc/MICC-F2000/"+wm_file[0])  #这一行中的文件夹前缀要看get_folder中随机出的wm名是否带文件夹路径
#         watermark = watermark1.resize((200,200)) #如果不需要resize就把这行注释掉并把上一行改为watermark
#         img_size = img.size
#         wm_size = watermark.size
        
#         # while img_size[0] < wm_size[0]:
#         #     watermark.resize(tuple(map(lambda x: int(x * 0.5), watermark.size)))
#         #     wm_size = watermark.size
#         # print('image size: ', img_size)
#         # w = img_size[0]-wm_size[0]
#         # h = img_size[1]-wm_size[1]
#         # if w<=0:
#         #     w = 1
#         # if h<=0 :
#         #     h = 1
#         # wm_position = (random.randrange(0,w,1),random.randrange(0,h,1))
#         wm_position = (0,0)

#         # wm_position = (img_size[0]-wm_size[0],img_size[1]-wm_size[1])
#         layer = Image.new('RGBA', img.size)  
#         layer.paste(watermark, wm_position)  
#         mark_img = Image.composite(layer, img, layer)
#         new_file_name = str(order)+'+'+img_file.split('/')[-1]
#         mark_img.save(save_path + new_file_name,quality=90,subsampling=0)
#     except Exception as e:
#         print(traceback.print_exc())

# get_folder("/home/dc/data/image_data/new200raw/10/","/home/dc/data/image_data/newnew200/10/",1)
# get_folder("/home/dc/data/image_data/new200raw/11/","/home/dc/data/image_data/newnew200/11/",1)
# get_folder("/home/dc/data/image_data/new200raw/12/","/home/dc/data/image_data/newnew200/12/",1)
# get_folder("/home/dc/data/image_data/new200raw/13/","/home/dc/data/image_data/newnew200/13/",1)
# get_folder("/home/dc/data/image_data/new200raw/14/","/home/dc/data/image_data/newnew200/14/",1)
# get_folder("/home/dc/data/image_data/new200raw/15/","/home/dc/data/image_data/newnew200/15/",1)
# get_folder("/home/dc/data/image_data/new200raw/16/","/home/dc/data/image_data/newnew200/16/",1)
# get_folder("/home/dc/data/image_data/new200raw/17/","/home/dc/data/image_data/newnew200/17/",1)
# get_folder("/home/dc/data/image_data/new200raw/18/","/home/dc/data/image_data/newnew200/18/",1)
# get_folder("/home/dc/data/image_data/new200raw/19/","/home/dc/data/image_data/newnew200/19/",1)
# get_folder("/home/dc/data/image_data/new200raw/20/","/home/dc/data/image_data/newnew200/20/",1)
# get_folder("/home/dc/data/image_data/new200raw/21/","/home/dc/data/image_data/newnew200/21/",1)
# get_folder("/home/dc/data/image_data/new200raw/22/","/home/dc/data/image_data/newnew200/22/",1)
# get_folder("/home/dc/data/image_data/new200raw/23/","/home/dc/data/image_data/newnew200/23/",1)
# get_folder("/home/dc/data/image_data/new200raw/24/","/home/dc/data/image_data/newnew200/24/",1)
# get_folder("/home/dc/data/image_data/new200raw/25/","/home/dc/data/image_data/newnew200/25/",1)
# get_folder("/home/dc/data/image_data/new200raw/26/","/home/dc/data/image_data/newnew200/26/",1)
# get_folder("/home/dc/data/image_data/new200raw/27/","/home/dc/data/image_data/newnew200/27/",1)
# get_folder("/home/dc/data/image_data/new200raw/28/","/home/dc/data/image_data/newnew200/28/",1)
# get_folder("/home/dc/data/image_data/new200raw/29/","/home/dc/data/image_data/newnew200/29/",1)
# get_folder("/home/dc/data/image_data/new200raw/30/","/home/dc/data/image_data/newnew200/30/",1)
# get_folder("/home/dc/data/image_data/new200raw/31/","/home/dc/data/image_data/newnew200/31/",1)
# get_folder("/home/dc/data/image_data/new200raw/32/","/home/dc/data/image_data/newnew200/32/",1)
# get_folder("/home/dc/data/image_data/new200raw/33/","/home/dc/data/image_data/newnew200/33/",1)
# get_folder("/home/dc/data/image_data/new200raw/34/","/home/dc/data/image_data/newnew200/34/",1)
# get_folder("/home/dc/data/image_data/new200raw/35/","/home/dc/data/image_data/newnew200/35/",1)
# get_folder("/home/dc/data/image_data/new200raw/36/","/home/dc/data/image_data/newnew200/36/",1)
# get_folder("/home/dc/data/image_data/new200raw/37/","/home/dc/data/image_data/newnew200/37/",1)
# get_folder("/home/dc/data/image_data/new200raw/38/","/home/dc/data/image_data/newnew200/38/",1)
# get_folder("/home/dc/data/image_data/new200raw/39/","/home/dc/data/image_data/newnew200/39/",1)
# get_folder("/home/dc/data/image_data/new200raw/40/","/home/dc/data/image_data/newnew200/40/",1)
# get_folder("/home/dc/data/image_data/new200raw/41/","/home/dc/data/image_data/newnew200/41/",1)
# get_folder("/home/dc/data/image_data/new200raw/42/","/home/dc/data/image_data/newnew200/42/",1)
# get_folder("/home/dc/data/image_data/new200raw/43/","/home/dc/data/image_data/newnew200/43/",1)
# get_folder("/home/dc/data/image_data/new200raw/44/","/home/dc/data/image_data/newnew200/44/",1)
# get_folder("/home/dc/data/image_data/new200raw/45/","/home/dc/data/image_data/newnew200/45/",1)
# get_folder("/home/dc/data/image_data/new200raw/46/","/home/dc/data/image_data/newnew200/46/",1)
# get_folder("/home/dc/data/image_data/new200raw/47/","/home/dc/data/image_data/newnew200/47/",1)
# get_folder("/home/dc/data/image_data/new200raw/48/","/home/dc/data/image_data/newnew200/48/",1)
# get_folder("/home/dc/data/image_data/new200raw/49/","/home/dc/data/image_data/newnew200/49/",1)

# get_folder("/home/dc/data/image_data/imagenet/0/","/home/dc/data/image_data/holmes/0/",1)
# get_folder("/home/dc/data/image_data/imagenet/1/","/home/dc/data/image_data/holmes/1/",1)
# get_folder("/home/dc/data/image_data/imagenet/2/","/home/dc/data/image_data/holmes/2/",1)
# get_folder("/home/dc/data/image_data/imagenet/3/","/home/dc/data/image_data/holmes/3/",1)
# get_folder("/home/dc/data/image_data/imagenet/4/","/home/dc/data/image_data/holmes/4/",1)
# get_folder("/home/dc/data/image_data/imagenet/5/","/home/dc/data/image_data/holmes/5/",1)
# get_folder("/home/dc/data/image_data/imagenet/6/","/home/dc/data/image_data/holmes/6/",1)
# get_folder("/home/dc/data/image_data/imagenet/7/","/home/dc/data/image_data/holmes/7/",1)
# get_folder("/home/dc/data/image_data/imagenet/8/","/home/dc/data/image_data/holmes/8/",1)
# get_folder("/home/dc/data/image_data/imagenet/9/","/home/dc/data/image_data/holmes/9/",1)

# get_folder("/home/dc/data/image_data/VOC2012/JPEGImages/","/home/dc/data/image_data/edt-v-5-check/0/",4) //95
get_folder("/home/dc/data/image_data/VOC2012/JPEGImages/","/home/dc/data/image_data/edt-v-4-check/0/",4) //75

# get_folder("/home/dc/data/image_data/VOC2012/JPEGImages/","/home/dc/data/image_data/WMVOC2012_600X200/0/",1)

# get_folder("/home/dc/data/image_data/VOC2012PARTS/0/","/home/dc/data/image_data/wmvoc4/0/",4)
# get_folder("/home/dc/data/image_data/VOC2012PARTS/1/","/home/dc/data/image_data/wmvoc4/1/",4)
# get_folder("/home/dc/data/image_data/VOC2012PARTS/2/","/home/dc/data/image_data/wmvoc4/2/",4)
# get_folder("/home/dc/data/image_data/VOC2012PARTS/3/","/home/dc/data/image_data/wmvoc4/3/",4)
# get_folder("/home/dc/data/image_data/VOC2012PARTS/4/","/home/dc/data/image_data/wmvoc4/4/",4)
# get_folder("/home/dc/data/image_data/VOC2012PARTS/5/","/home/dc/data/image_data/wmvoc4/5/",4)
# get_folder("/home/dc/data/image_data/VOC2012PARTS/6/","/home/dc/data/image_data/wmvoc4/6/",4)
# get_folder("/home/dc/data/image_data/VOC2012PARTS/7/","/home/dc/data/image_data/wmvoc4/7/",4)
# get_folder("/home/dc/data/image_data/VOC2012PARTS/8/","/home/dc/data/image_data/wmvoc4/8/",4)
# get_folder("/home/dc/data/image_data/VOC2012PARTS/9/","/home/dc/data/image_data/wmvoc4/9/",4)

# get_folder("/home/dc/data/image_data/VOC2012PARTS/0/","/home/dc/data/image_data/wmvoc8/0/",8)
# get_folder("/home/dc/data/image_data/VOC2012PARTS/1/","/home/dc/data/image_data/wmvoc8/1/",8)
# get_folder("/home/dc/data/image_data/VOC2012PARTS/2/","/home/dc/data/image_data/wmvoc8/2/",8)
# get_folder("/home/dc/data/image_data/VOC2012PARTS/3/","/home/dc/data/image_data/wmvoc8/3/",8)
# get_folder("/home/dc/data/image_data/VOC2012PARTS/4/","/home/dc/data/image_data/wmvoc8/4/",8)
# get_folder("/home/dc/data/image_data/VOC2012PARTS/5/","/home/dc/data/image_data/wmvoc8/5/",8)
# get_folder("/home/dc/data/image_data/VOC2012PARTS/6/","/home/dc/data/image_data/wmvoc8/6/",8)
# get_folder("/home/dc/data/image_data/VOC2012PARTS/7/","/home/dc/data/image_data/wmvoc8/7/",8)
# get_folder("/home/dc/data/image_data/VOC2012PARTS/8/","/home/dc/data/image_data/wmvoc8/8/",8)
# get_folder("/home/dc/data/image_data/VOC2012PARTS/9/","/home/dc/data/image_data/wmvoc8/9/",8)

# get_folder("/home/dc/data/image_data/VOC2012PARTS/0/","/home/dc/data/image_data/wmvoc10/0/",10)
# get_folder("/home/dc/data/image_data/VOC2012PARTS/1/","/home/dc/data/image_data/wmvoc10/1/",10)
# get_folder("/home/dc/data/image_data/VOC2012PARTS/2/","/home/dc/data/image_data/wmvoc10/2/",10)
# get_folder("/home/dc/data/image_data/VOC2012PARTS/3/","/home/dc/data/image_data/wmvoc10/3/",10)
# get_folder("/home/dc/data/image_data/VOC2012PARTS/4/","/home/dc/data/image_data/wmvoc10/4/",10)
# get_folder("/home/dc/data/image_data/VOC2012PARTS/5/","/home/dc/data/image_data/wmvoc10/5/",10)
# get_folder("/home/dc/data/image_data/VOC2012PARTS/6/","/home/dc/data/image_data/wmvoc10/6/",10)
# get_folder("/home/dc/data/image_data/VOC2012PARTS/7/","/home/dc/data/image_data/wmvoc10/7/",10)
# get_folder("/home/dc/data/image_data/VOC2012PARTS/8/","/home/dc/data/image_data/wmvoc10/8/",10)
# get_folder("/home/dc/data/image_data/VOC2012PARTS/9/","/home/dc/data/image_data/wmvoc10/9/",10)

# get_folder("/home/dc/data/image_data/voc/0/","/home/dc/data/image_data/wmvoc18/0/",18)
# get_folder("/home/dc/data/image_data/voc/1/","/home/dc/data/image_data/wmvoc18/1/",18)
# get_folder("/home/dc/data/image_data/voc/2/","/home/dc/data/image_data/wmvoc18/2/",18)
# get_folder("/home/dc/data/image_data/voc/3/","/home/dc/data/image_data/wmvoc18/3/",18)
# get_folder("/home/dc/data/image_data/voc/4/","/home/dc/data/image_data/wmvoc18/4/",18)
# get_folder("/home/dc/data/image_data/voc/5/","/home/dc/data/image_data/wmvoc18/5/",18)
# get_folder("/home/dc/data/image_data/voc/6/","/home/dc/data/image_data/wmvoc18/6/",18)
# get_folder("/home/dc/data/image_data/voc/7/","/home/dc/data/image_data/wmvoc18/7/",18)
# get_folder("/home/dc/data/image_data/voc/8/","/home/dc/data/image_data/wmvoc18/8/",18)
# get_folder("/home/dc/data/image_data/voc/9/","/home/dc/data/image_data/wmvoc18/9/",18)