#ifndef MYTINYSTL_ALLOCATOR_H_
#define MYTINYSTL_ALLOCATOR_H_

// 这个头文件包含一个模板类 allocator，用于管理内存的分配、释放，对象的构造、析构

#include "construct.h"
#include "util.h"
#include <memory>

namespace mystl
{

template <typename T>
inline T* allocate(size_t n, T* ptr)
{
    if(n == 0)
        return nullptr;
    T* tmp = (T*)::operator new(n*sizeof(T));
    if(tmp == 0)
    {
        std::cout << "out of memory" << std::endl;
        exit(1);
    }
    return tmp;
}

template <typename T>
inline void deallocate(T* ptr)
{
    if(ptr == nullptr)
        return;
    ::operator delete(ptr);
}


template <class T>
class allocator
{
public:
    typedef T            value_type;
    typedef T*           pointer;
    typedef const T*     const_pointer;
    typedef T&           reference;
    typedef const T&     const_reference;
    typedef size_t       size_type;         //size_t:unsigned __int64
    typedef ptrdiff_t    difference_type;   //ptrdiff_t:__int64

public:
    static pointer allocate()
    {
        return mystl::allocate(1, pointer(0));
    }

    static pointer allocate(size_type n)
    {
        return mystl::allocate(n, pointer(0));
    }

    static void deallocate(pointer ptr)
    {
        mystl::deallocate(ptr);
    }


    //construct and destroy
    static void construct(pointer ptr)
    {
        mystl::construct(ptr);
    }

    static void construct(pointer ptr, const_reference value)
    {
        mystl::construct(ptr, value);
    }
    static void construct(pointer ptr, T&& value)
    {
        mystl::construct(ptr, std::move(value)); //添加std::move后成功 why??? 在右值具有变量名绑定后将变为左值
    }

    template <class... Args>
    static void construct(pointer ptr, Args&& ...args)
    {
        mystl::construct(ptr, std::forward<Args>(args)...);
    }

    static void destroy(pointer ptr)
    {
        mystl::destroy(ptr);
    }

    static void destroy(pointer start, pointer end)
    {
        mystl::destroy(start, end);
    }
}; //class allocator


// 在外面定义类方法，也需要类的模板
// template <class T>
// allocator<T>::pointer allocator<T>::allocate()
// {
// }



}   // mystl

#endif 