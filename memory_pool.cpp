#include <iostream>




enum {__ALIGN = 8};         //内存块上调上界
enum {__MAX_BYTES = 128};   //内存块大小上界
enum {__NFREELISTS = __MAX_BYTES / __ALIGN};    //free-lists 个数

// 以下是第二级配置器
// 无<template类别参数> 且第二参数完全没派上用场
template <bool threads, int inst>
class __default_alloc_template
{
private:
    // ROUND_UP() 将bytes上调至8的倍数
    static size_t ROUND_UP(size_t bytes)
    {
        return ((bytes + __ALIGN - 1) & ~(__ALIGN - 1));
    }
    
    // free-list的结构体
    union obj
    {
        union obj* free_list_link;
        char client_data[1];
    };

    // 根据给定区块大小 决定使用第n号free-list n从1开始算
    static size_t FREELIST_INDEX(size_t bytes)
    {
        return ((bytes + __ALIGN - 1) / (__ALIGN - 1));
    }

    // 返回一个大小为n的对象，并可能将该区块加入到free-list
    static void* refill(size_t n);

    // 配置nobjs个大小为"size"的区块
    // 配置区块如果不便，nobjs可能会降低
    static char* chunk_alloc(size_t size, int &nobjs);

public:
    static void* allocate(size_t n);
    static void* deallocate(void* p, size_t n);
    static void* reallocate(void* p, size_t old_sz, size_t new_sz);

private:
    // 16个free-lists
    static obj* volatile free_list[__NFREELISTS];

    // Chunk allocation state
    static char* start_free;
    static char* end_free;
    static size_t heap_size;
};


template <bool threads, int inst>
char *__default_alloc_template<threads, inst>::start_free = 0;

template <bool threads, int inst>
char *__default_alloc_template<threads, inst>::end_free = 0;

template <bool threads, int inst>
size_t __default_alloc_template<threads, inst>::heap_size = 0;

template <bool threads, int inst>
__default_alloc_template<threads, inst>::obj * volatile
__default_alloc_template<threads, inst>::free_list[__NFREELISTS] = {0};

static void* allocate(size_t n)
{
    obj* volatile * my_free_list;
    obj* result;

    // 大于128使用第一级配置器
    if(n > (size_t) __MAX_BYTES)
        return (malloc_alloc::allocate(n));

    // 寻找16个free list中的一个
    my_free_list = free_list + FREELIST_INDEX(n);
    result = *my_free_list;
    if (result == 0)
    {
        void* r = refill(ROUND_UP(n));
        return r;
    }

    //调整free list
    *my_free_list = result->free_list_link;
    return result;
};


static void deallocate(void *p, size_t n)
{
    obj* q = (obj*)p;
    obj* volatile * my_free_list;

    //大于128使用第一级配置器
    if(n > (size_t) __MAX_BYTES)
    {
        malloc_alloc::deallocate(p, n);
        return;
    }

    my_free_list = free_list + FREELIST_INDEX(n);
    
    //调整free list 回收区块
    q->free_list_link = *my_free_list;
    *my_free_list = q;
}



//refill
//返回一个大小为n的对象，并且有时候会为free list增加节点 
template <bool threads, int inst>
void * __default_alloc_template<threads, inst>::refill(size_t n)
{
    int nobjs = 20;
    // 调用chunk_alloc() 尝试分配nobjs个区块
    char* chunk = chunk_alloc(n, nobjs);
    obj* volatile * my_free_list;
    obj* result;
    obj* current_obj, * next_obj;
    int i;

    // 如果只有一个区块 直接给分配者使用
    if(1 == nobjs) return(chunk);

    // 否则调整free_list, 纳入新节点
    my_free_list = free_list + FREELIST_INDEX(n);

    // 在chunk空间建立free_list
    result = (obj*)chunk;
    *my_free_list = next_obj = (obj*)(chunk + n);

    for(i = 1; ; i++)
    {
        current_obj = next_obj;
        next_obj = (obj*)((char*)next_obj + n);
        if (nobjs - 1 == i)
        {
            current_obj->free_list_link = 0;
            break;
        }
        else
        {
            current_obj->free_list_link = next_obj;
        }
    }

    return result;
}


//从内存池中取空间给free list使用 是chunk_alloc()的工作
template <bool threads, int inst>
char* __default_alloc_template<threads, inst>::
chunk_alloc(size_t size, int& nobjs)
{
    char* result;
    size_t total_bytes = size * nobjs;
    size_t bytes_left = end_free - start_free;  // 内存池剩余空间

   
    if(bytes_left >= total_bytes)   // 剩余空间满足需求量
    {
        result = start_free;
        start_free += total_bytes;
        return result;
    }
    else if(bytes_left >= size)      // 剩余空间满足一个size
    {
        nobjs = bytes_left / size;
        total_bytes = size * nobjs;
        result = start_free;
        start_free ++ total_bytes;
        return result;
    }
    else                            // 剩余空间小于一个size
    {
        size_t bytes_to_get = 2 * total_bytes + ROUND_UP(heap_size >> 4);

        //将内存池剩余空间进行分配
        if (bytes_left > 0)
        {
            obj* volatile * my_free_list = free_list + FREELIST_INDEX(bytes_left);
            ((obj*)start_free)->free_list_link = *my_free_list;
            *my_free_list = (obj*)start_free;
        }
    
        //向heap空间申请新的内存
        start_free = (char*)malloc(bytes_to_get);

        //heap空间不足，malloc()失败
        if(0 == start_free)
        {
            //试着利用已有的空间进行分配: 尚有未有区块，且区块足够大的free list
            for(i = size; i <= __MAX_BYTES; i += __ALIGN)
            {
                my_free_list = free_list + FREELIST_INDEX(i);
                p = *my_free_list;
                if(0 != p)
                {
                    *my_free_list = p->free_list_link;
                    start_free = (char*)p;
                    end_free = start_free + i;
                    return chunk_alloc(size, nobjs);
                }
            } 

            //如果出现意外，即到处都没有内存
            end_free = 0;
            //调用第一级分配器，这会抛出异常，或内存不足时获得改善
            start_free = (char*)malloc_alloc::allocate(bytes_to_get);
        }

        heap_size += bytes_to_get;
        end_free = start_free + bytes_to_get;
        //递归调用自己， 为了修正nobjs
        return chunk_alloc(size, nobjs);
    }


}
