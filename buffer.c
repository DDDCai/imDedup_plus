/*
 * @Author: Cai Deng
 * @Date: 2021-01-13 15:08:44
 * @LastEditors: Cai Deng
 * @LastEditTime: 2021-03-14 13:12:45
 * @Description: 
 */

#include "buffer.h"

static void move_action(buf_node *item, Buffer *buf, uint64_t(*data_func)(void *p), void(*free_func)(void *p));

static void delete_from_top_action(Buffer *buf, void(*free_func)(void *p))
{
    buf_node    *ptr    =   buf->head;
    buf->head   =   ptr->next;
    buf->head->pre  =   NULL;
    buf->size   +=  ptr->size;
    pthread_mutex_lock(&ptr->mutex);
    if(!ptr->link)
        free_func(ptr->data);
    else 
        move_action(ptr, buf, NULL, free_func);
    pthread_mutex_unlock(&ptr->mutex);
}

static void move_action(buf_node *item, Buffer *buf, uint64_t(*data_func)(void *p), void(*free_func)(void *p))
{
    uint64_t extraSize;
    if(data_func)
        extraSize = data_func(item);
    else 
        extraSize = item->size;
    if(extraSize)
    {
        while(buf->size < extraSize)
            delete_from_top_action(buf, free_func);
        buf->size -= extraSize;
    }
    else if(item != buf->tail)
    {
        if(item->pre)
            item->pre->next = item->next;
        else 
            buf->head = item->next;
        item->next->pre = item->pre;
    }
    else return ;
    buf->tail->next = item;
    item->pre = buf->tail;
    item->next = NULL;
    buf->tail = item;
}

void insert_to_buffer(buf_node *item, Buffer *buf, void(*free_func)(void *p))
{
    pthread_mutex_lock(&buf->mutex);
    while(buf->size < item->size)
        delete_from_top_action(buf, free_func);
    if(buf->head)
    {
        buf->tail->next =   item;
        item->pre   =   buf->tail;
    }
    else 
    {
        buf->head   =   item;
        item->pre   =   NULL;
    }
    item->next  =   NULL;
    buf->tail   =   item;
    buf->size -= item->size;
    pthread_mutex_unlock(&buf->mutex);
}

void move_in_buffer(buf_node *item, Buffer *buf, uint64_t(*data_func)(void *p), void(*free_func)(void *p))
{
    pthread_mutex_lock(&buf->mutex);
    // if(buf->size < START_TO_MOVE)
        move_action(item, buf, data_func, free_func);
    pthread_mutex_unlock(&buf->mutex);
}