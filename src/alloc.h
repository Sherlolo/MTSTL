#ifndef MYYSTL_ALLOC_H_
#define MYYSTL_ALLOC_H_

#if 0
# include <new>
# define __THROW_BAD_ALLOC throw bad_alloc
#elif !defined(__THROW_BAD_ALLOC) 
# include <iostream>
# define __THROW_BAD_ALLOC std::cerr << "out of memory" << std::endl; exit(1)
#endif

#include <new>
#include <cstddef>
#include <cstdio>
#include "construct.h"

namespace mystl
{

//the first alloc
template <int inst>
class __malloc_alloc_template
{
private:
  //oom: out of memory
  static void *oom_malloc(size_t);
  static void *oom_realloc(void *, size_t);
  static void (* __malloc_alloc_oom_handler)();

public:

  static void* allocate(size_t n)
  {
    void* result = malloc(n);
    if(0 == result)
      result = oom_malloc(n);
    return result;
  }

  static void deallocate(void* p, size_t)
  {
    free(p);
  }

  static void* reallocate(void* p, size_t old_size, size_t new_size)
  {
    void* result = realloc(p, new_size);
    if(0 == result)
      result = oom_realloc(p, new_size);
    return result;
  }

  // out of memory handler
  // To set my handler
  static void (*set_malloc_handler(void (*f()))) ()
  {
    void (*old)() = __malloc_alloc_oom_handler;
    __malloc_alloc_oom_handler = f;
    return (old);
  }

};  // class __malloc_alloc_template


template <int inst>
void (* __malloc_alloc_template<inst>::__malloc_alloc_oom_handler)() = 0;

template <int inst>
void* __malloc_alloc_template<inst>::oom_malloc(size_t n)
{
  void (* my_malloc_handler)();
  void* result;
  for(;;)
  {
    my_malloc_handler = __malloc_alloc_oom_handler;
    if(0 == my_malloc_handler)
      __THROW_BAD_ALLOC;
    (*my_malloc_handler)();
    result = malloc(n);
    if(result)
      return result;
  }
}

template <int inst>
void* __malloc_alloc_template<inst>::oom_realloc(void* p, size_t n)
{
  void (* my_malloc_handler)();
  void* result;
  for(;;)
  {
    my_malloc_handler = __malloc_alloc_oom_handler;
    if(0 == my_malloc_handler)
      __THROW_BAD_ALLOC;
    (*my_malloc_handler)();
    result = realloc(p, n);
    if(result)
      return result;
  }
}

typedef __malloc_alloc_template<0> malloc_alloc;


//the second alloc
enum { __ALIGN = 8};
enum { __MAX_BYTES = 128};
enum { __NFREELISTS = __MAX_BYTES/__ALIGN};

template <bool threads, int inst>
class __default_alloc_template
{
private:
  static size_t ROUND_UP(size_t bytes)  //up to 8*n
  {
    return (bytes + __ALIGN - 1) & ~(__ALIGN - 1);
  }

  union obj
  {
    union obj* free_list_link;
    char client_data[1];
  };

  static obj * volatile free_list[__NFREELISTS]; //volatile: the data must be in memory
  static size_t FREELIST_INDEX(size_t bytes)
  {
    return (bytes + __ALIGN - 1) / __ALIGN - 1;
  }

  static void* refill(size_t n);
  static char* chunk_alloc(size_t size, int& nobjs);

  static char* start_free;
  static char* end_free;
  static size_t heap_size; //heap_size: the size of memory from heap

public:
  static void* allocate(size_t n)
  {
    obj* volatile * my_free_list;
    obj* result;
    if(n > (size_t)__MAX_BYTES)
      return malloc_alloc::allocate(n);
    my_free_list = free_list + FREELIST_INDEX(n);
    result = *my_free_list;
    if(result == 0)
    {
      void* r = refill(ROUND_UP(n));
      return r;
    }
    *my_free_list = result->free_list_link;
    return result;
  }

  static void deallocate(void* p, size_t n)
  {
    obj* q = (obj*) p;
    obj* volatile * my_free_list;

    if(n > (size_t) __MAX_BYTES)
    {
      malloc_alloc::deallocate(p, n);
      return;
    }

    my_free_list = free_list + FREELIST_INDEX(n);
    q->free_list_link = *my_free_list;
    *my_free_list = q;
  }

  static void* reallocate(void* p, size_t old_size, size_t new_size)
  {
    size_t copy_size;
    obj* result;
    if(old_size > (size_t)__MAX_BYTES && new_size > (size_t)__MAX_BYTES)
      return malloc_alloc::reallocate(p, old_size, new_size);
    if(ROUND_UP(old_size) == ROUND_UP(new_size))
      return p;

    result = allocate(new_size);
    copy_size = new_size > old_size? old_size : new_size;
    memcpy(result, p, copy_size);
    deallocate(p, old_size); 
    return result;
  }

};  // class __default_alloc_template

template <bool threads, int inst>
char* __default_alloc_template<threads, inst>::start_free = 0;

template <bool threads, int inst>
char* __default_alloc_template<threads, inst>::end_free = 0;

template <bool threads, int inst>
size_t __default_alloc_template<threads, inst>::heap_size = 0;

template <bool threads, int inst>
typename __default_alloc_template<threads, inst>::obj* volatile
__default_alloc_template<threads, inst>::free_list[__NFREELISTS] = 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};


//refill and chunk_alloc
template <bool threads, int inst>
void* __default_alloc_template<threads, inst>::refill(size_t n)
{
  int nobjs = 20;
  char* chunk = chunk_alloc(n, nobjs);  //nobjs is pass by reference
  obj* volatile * my_free_list;
  obj* result;
  obj* current_obj, * next_obj;
  int i;

  if(1 == nobjs)  return chunk;
  my_free_list = free_list + FREELIST_INDEX(n);

  result = (obj*)chunk;
  *my_free_list = next_obj = (obj*)(chunk + n);

  for(i = 1; ; ++i) //form i=1 start, i = 0 will return to client
  {
    current_obj = next_obj;
    next_obj = (obj*)((char* )next_obj + n);
    if(nobjs - 1 == i)
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

//chunk_alloc
template <bool threads, int inst>
char* __default_alloc_template<threads, inst>::chunk_alloc(size_t size, int& nobjs)
{
  char* result;
  size_t total_bytes = size * nobjs;
  size_t bytes_left = end_free - start_free;

  if(bytes_left >= total_bytes)
  {
    result = start_free;
    start_free += total_bytes;
    return result;
  }
  else if(bytes_left >= size)
  {
    nobjs = bytes_left / size;
    total_bytes = size * nobjs;
    result = start_free;
    start_free += total_bytes;
    return result;
  }
  else
  {
    size_t bytes_to_get = 2 * total_bytes + ROUND_UP(heap_size >> 4);

    // use bytes_left
    if(bytes_left > 0)
    {
      obj* volatile * my_free_list = free_list + FREELIST_INDEX(bytes_left);
      ((obj*)start_free)->free_list_link = *my_free_list;
      *my_free_list = (obj*)start_free;
    }

    start_free = (char*)malloc(bytes_to_get);

    //find my_list whether has memory
    if(0 == start_free)
    {
      int i;
      obj* volatile * my_free_list, *p;
      for(i = size; i <= __MAX_BYTES; i += __ALIGN)
      {
        my_free_list = free_list + FREELIST_INDEX(i);
        p = *my_free_list;
        if(0 != p)
        {
          *my_free_list = p->free_list_link;
          start_free = (char*)p;
          end_free = start_free + i;
          return chunk_alloc(size, nobjs);  //to correct nobjs
        }
      }
      end_free = 0;
      start_free = (char*)malloc_alloc::allocate(bytes_to_get);   //to use malloc_alloc_handle
    }

    heap_size += bytes_to_get;
    end_free = start_free + bytes_to_get;
    return chunk_alloc(size, nobjs);  //to correct nobjs
  }
}

typedef __default_alloc_template<true, 0> default_alloc;

//the actual use allocator
template<class T, class Alloc>
class simple_alloc {

public:
    static T *allocate(size_t n)
                { return 0 == n? 0 : (T*) Alloc::allocate(n * sizeof (T)); }
    static T *allocate(void)
                { return (T*) Alloc::allocate(sizeof (T)); }
    static void deallocate(T *p, size_t n)
                { if (0 != n) Alloc::deallocate(p, n * sizeof (T)); }
    static void deallocate(T *p)
                { Alloc::deallocate(p, sizeof (T)); }

    //construct and destroy
    static void construct(T* ptr)
    {
        mystl::construct(ptr);
    }

    static void construct(T* ptr, const T& value)
    {
        mystl::construct(ptr, value);
    }
    static void construct(T* ptr, T&& value)
    {
        mystl::construct(ptr, std::move(value)); //添加std::move后成功 why??? 在右值具有变量名绑定后将变为左值
    }

    template <class... Args>
    static void construct(T* ptr, Args&& ...args)
    {
        mystl::construct(ptr, std::forward<Args>(args)...);
    }

    static void destroy(T* ptr)
    {
        mystl::destroy(ptr);
    }

    static void destroy(T* start, T* end)
    {
        mystl::destroy(start, end);
    }
};

} // namespace mystl

#endif // !MYTINYSTL_ALLOC_H_
