#ifndef MYSTL_UNINITIALIZED_H_
#define MYSTL_UNINITIALIZED_H_

#include <new>
#include <memory>
#include <type_traits>
#include "type_traits.h"

namespace mystl
{

// to POD or 
// destructor is trivial

template <class InputIterator, class ForwardIterator>
inline ForwardIterator __uninitialized_copy_aux(InputIterator first, InputIterator last, ForwardIterator result, mystl::m_true_type)
{
    return std::copy(first, last, result);  //to use stl algorithm
}

template <class InputIterator, class ForwardIterator>
inline ForwardIterator __uninitialized_copy_aux(InputIterator first, InputIterator last, ForwardIterator result, mystl::m_false_type)
{
    ForwardIterator cur = result;
    for(; first != last; ++first, ++cur)
    {
        mystl::construct(&*cur, *first);
    }
    return cur;
}


// __uninitialized_copy
template <class InputIterator, class ForwardIterator, class T>
inline ForwardIterator __uninitialized_copy(InputIterator first, InputIterator last, ForwardIterator result, T*)
{
    return __uninitialized_copy_aux(first, last, result, mystl::is_POD<T>());
}

template <class InputIterator, class ForwardIterator>
inline ForwardIterator uninitialized_copy(InputIterator first, InputIterator last, ForwardIterator result)
{
    return __uninitialized_copy(first, last, result, mystl::value_type(result));
}

// __uninitialized_copy to char* 
inline char* uninitialized_copy(const char* first, const char* last, char* result)
{
    memmove(result, first, last - first);
    return result + (last - first);
}

// __uninitialized_copy to wchar_t* 
inline wchar_t* uninitialized_copy(const wchar_t* first, const wchar_t* last, wchar_t* result)
{
    memmove(result, first, last - first);
    return result + (last - first);
}

// uninitialized_fill

// to POD or 
// destructor is trivial
template <class ForwardIterator, class T>
inline void __uninitialized_fill_aux(ForwardIterator first, ForwardIterator last, const T& x, mystl::m_true_type)
{
    std::fill(first, last, x);  //use stl algorithm
}

template <class ForwardIterator, class T>
inline void __uninitialized_fill_aux(ForwardIterator first, ForwardIterator last, const T& x, mystl::m_false_type)
{
    ForwardIterator cur = first;
    for(; first != last; ++first, ++cur)
    {
        mystl::construct(&*cur, x);
    }
}

// uninitialized_fill
template <class ForwardIterator, class T, class T1>
inline void __uninitialized_fill(ForwardIterator first, ForwardIterator last, const T& x, T1*)
{
    __uninitialized_fill_aux(first, last, x, mystl::is_POD<T1>());
}

template <class ForwardIterator, class T>
inline void uninitialized_fill(ForwardIterator first, ForwardIterator last, const T& x)
{
    __uninitialized_fill(first, last, x, mystl::value_type(first));
}


// uninitialized_fill_n

// to POD or 
// destructor is trivial
template <class ForwardIterator, class Size, class T>
inline void __uninitialized_fill_n_aux(ForwardIterator first, Size n, const T& x, mystl::m_true_type)
{
    std::fill_n(first, n, x);  //use stl algorithm
}

template <class ForwardIterator, class Size, class T>
inline void __uninitialized_fill_n_aux(ForwardIterator first, Size n, const T& x, mystl::m_false_type)
{
    ForwardIterator cur = first;
    for(; n > 0; --n, ++cur)
    {
        mystl::construct(&*cur, x);
    }
}

// uninitialized_fill_n
template <class ForwardIterator, class Size, class T, class T1>
inline void __uninitialized_fill_n(ForwardIterator first, Size n, const T& x, T1*)
{
    __uninitialized_fill_n_aux(first, n, x, mystl::is_POD<T1>());
}

template <class ForwardIterator, class Size, class T>
inline void uninitialized_fill_n(ForwardIterator first, Size n, const T& x)
{
    __uninitialized_fill_n(first, n, x, mystl::value_type(first));
}

// // uninitialized_move
// // 把 [first, last)上的内容移动到result为起始处的空间 返回移动结束的位置
// template <class InputIter, class ForwardIter>
// ForwardIter unchecked_uninit_move(InputIter first, InputIter last, ForwardIter result, std::true_type)
// {
//     return mystl::move(first, last, result);
// }

// template <class InputIter, class ForwardIter>
// ForwardIter unchecked_uninit_move(InputIter first, InputIter last, ForwardIter result, std::false_type)
// {
//     ForwardIter cur = result;
//     try
//     {
//         for(; first != last; ++first, ++cur)
//         {
//             mystl::construct(&*cur, mystl::move(*first));
//         }
//     }
//     catch(...)
//     {
//         mystl::destroy(result, cur);
//     }
//     return cur;
// }

// template <class InputIter, class ForwardIter>
// ForwardIter uninitialized_move(InputIter first, InputIter last, ForwardIter result)
// {
//     return mystl::unchecked_uninit_move(first, last, result, 
//                                         std::is_trivially_move_assignable<mystl::iterator_traits<InputIter>::value_type>());
// }

// template <class InputIter, class Size, class ForwardIter>
// ForwardIter 
// unchecked_uninit_move_n(InputIter first, Size n, ForwardIter result, std::true_type)
// {
//   return mystl::move(first, first + n, result);
// }

// template <class InputIter, class Size, class ForwardIter>
// ForwardIter
// unchecked_uninit_move_n(InputIter first, Size n, ForwardIter result, std::false_type)
// {
//   auto cur = result;
//   try
//   {
//     for (; n > 0; --n, ++first, ++cur)
//     {
//       mystl::construct(&*cur, mystl::move(*first));
//     }
//   }
//   catch (...)
//   {
//     for (; result != cur; ++result)
//       mystl::destroy(&*result);
//     throw;
//   }
//   return cur;
// }

// template <class InputIter, class Size, class ForwardIter>
// ForwardIter uninitialized_move_n(InputIter first, Size n, ForwardIter result)
// {
//   return mystl::unchecked_uninit_move_n(first, n, result,
//                                         std::is_trivially_move_assignable<
//                                         typename iterator_traits<InputIter>::
//                                         value_type>{});
// }

} // namespace mystl



#endif