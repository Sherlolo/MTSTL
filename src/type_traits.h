#ifndef MYSTL_TYPE_TRAITS_H_
#define MYSTL_TYPE_TRAITS_H_

#include <type_traits>

// define __type_traits for other type
// __STL_TEMPLATE_NULL struct __type_traits<unsigned long> {
//    typedef __true_type    has_trivial_default_constructor;
//    typedef __true_type    has_trivial_copy_constructor;
//    typedef __true_type    has_trivial_assignment_operator;
//    typedef __true_type    has_trivial_destructor;
//    typedef __true_type    is_POD_type;
// };

namespace mystl { 

template <class T, T v>
struct m_integral_constant
{
    static constexpr T value = v;
};

template <bool b>
using m_bool_constant = m_integral_constant<bool, b>;

typedef m_bool_constant<true> m_true_type;
typedef m_bool_constant<false> m_false_type;

/*****************************************************************************************/
// type traits

// is_pair
template <class T1, class T2>
struct pair;

template <class T>
struct is_pair : mystl::m_false_type {};

template <class T1, class T2>
struct is_pair<mystl::pair<T1, T2>> : mystl::m_true_type {};

//is_POD std::is_pod<T>::value
template <class T>
struct is_POD : mystl::m_bool_constant<std::is_pod<T>::value> {};

}   //namespace mystl

#endif // !MYTINYSTL_TYPE_TRAITS_H_