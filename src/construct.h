#ifndef MYSTL_CONSTRUCT_H_
#define MYSTL_CONSTRUCT_H_

// construct : 负责对象的构造
// destroy   : 负责对象的析构
#include <new>
#include "type_traits.h"
#include "iterator.h"
#include <memory>
#include <type_traits>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4100) // unused parameter
#endif // _MSC_VER

namespace mystl
{

// construct
template <typename T>
inline void construct(T* ptr)
{
    new(ptr) T(); 
}

template <typename T1, typename T2>
inline void construct(T1* ptr, const T2& value) //const T2& 可以接收一个右值作为参数，并延长右值的寿命为变量的作用寿命
{
    new(ptr) T2(value);
}

// template <typename T1, typename T2>
// inline void construct(T1* ptr, T2&& value)
// {
//     new(ptr) T2(std::move(value));
// }
// 
// error
// In file included from .\allocator.h:6,
//                  from .\test_main.cpp:1:
// .\construct.h: In instantiation of 'void mystl::construct(T1*, T2&&) [with T1 = int; T2 = int&]':
// .\allocator.h:76:25:   required from 'static void mystl::allocator<T>::construct(mystl::allocator<T>::pointer, T&&) [with T = int; mystl::allocator<T>::pointer = int*]'
// .\test_main.cpp:8:29:   required from here
// .\construct.h:34:5: error: new cannot be applied to a reference type
//    34 |     new(ptr) T2(value);



//import use method
template <typename T1, class... Args>
inline void construct(T1* ptr, Args&& ...args)
{
    new(ptr) T1(std::forward<Args>(args)...);
}

//destroy
template <typename T>
inline void destroy(T* ptr)
{
    ptr->~T();
}

//destroy to iterator
template <typename Iterator>
inline void destroy(Iterator first, Iterator last)
{
    // __destroy(first, last, iterator_traits<Iterator>::pointer(0));
    __destroy(first, last, mystl::value_type(first));
}

template <typename Iterator, typename T>
inline void __destroy(Iterator first, Iterator last, T*)
{
    __destroy_aux(first, last, std::is_trivially_destructible<T>());
}

template <typename Iterator>
inline void __destroy_aux(Iterator first, Iterator last, std::false_type)
{
    for(; first < last; ++first)
        destroy(&*first);
}

template <typename Iterator>
inline void __destroy_aux(Iterator first, Iterator last, std::true_type)
{
}

//destroy to char* or wchar_t*
inline void destroy(char*, char*){}

inline void destroy(wchar_t*, wchar_t*){}

}   // namesapce mystl


#ifdef _MSC_VER
#pragma warning(pop)
#endif // _MSC_VER

#endif // !MYSTL_CONSTRUCT_H_