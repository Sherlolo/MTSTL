#ifndef MYSTL_QUEUE_H_
#define MYSTL_QUEUE_H_

#include "deque.h"
#include "vector.h"
// #include "functional.h"
// #include "heap_algo.h"

namespace mystl
{

template <class T, class Container = mystl::deque<T>>
class queue
{
public:
    typedef Container container_type;
    typedef typename Container::value_type value_type;
    typedef typename Container::size_type size_type;
    typedef typename Container::reference reference;
    typedef typename Container::const_reference const_reference;

    static_assert(std::is_same<T, value_type>::value, "the value_type of container should be same with T");

private:
    container_type c_;

public:

    queue() = default;

    explicit queue(size_type n) : c_(n)
    {}

    queue(size_type n, const value_type& value) : c_(n, value)
    {}

    template <class IIter>
    queue(IIter first, IIter last) : c_(first, last)
    {}

    queue(std::initializer_list<T> ilist) : c_(ilist.begin(), ilist.end())
    {}

    queue(Container& c) : c_(c)
    {}

    queue(Container&& c) noexcept(std::is_nothrow_move_constructible<Container>::value)
    : c_(mystl::move(c))
    {}

    queue(const queue& rhs) : c_(rhs.c_)
    {}

    queue(queue&& rhs) noexcept(std::is_nothrow_move_constructible<Container>::value)
    :c_(mystl::move(rhs.c_)) 
    {}

    queue& operator(const queue& rhs)
    {
        c_ = rhs.c_;
        return *this;
    }

    queue& operator=(queue&& rhs) noexcept(std::is_nothrow_move_assignable<Container>::value)
    {
        c_ = mystl::move(rhs.c_);
        return *this;
    }

    queue& operator=(std::initializer_list<T> ilist)
    { 
        c_ = ilist; 
        return *this; 
    }

    ~queue() = default;

public:

    // 访问元素相关操作
    reference       front()       { return c_.front(); }
    const_reference front() const { return c_.front(); }
    reference       back()        { return c_.back(); }
    const_reference back()  const { return c_.back(); }

    // 容量相关操作
    bool      empty() const noexcept { return c_.empty(); }
    size_type size()  const noexcept { return c_.size(); }

    // 修改容器相关操作
    template <class ...Args>
    void emplace(Args&& ...args)
    { c_.emplace_back(mystl::forward<Args>(args)...); }

    void push(const value_type& value) 
    { c_.push_back(value); }
    void push(value_type&& value)      
    { c_.emplace_back(mystl::move(value)); }

    void pop()                         
    { c_.pop_front(); }

    void clear()         
    { 
        while (!empty())
        pop(); 
    }

    void swap(queue& rhs) noexcept(noexcept(mystl::swap(c_, rhs.c_)))
    { mystl::swap(c_, rhs.c_); }

    friend bool operator==(const queue& lhs, const queue& rhs) { return lhs.c_ == rhs.c_; }
    friend bool operator< (const queue& lhs, const queue& rhs) { return lhs.c_ <  rhs.c_; }


}; // class queu

/ 重载比较操作符
template <class T, class Container>
bool operator==(const queue<T, Container>& lhs, const queue<T, Container>& rhs)
{
  return lhs == rhs;
}

template <class T, class Container>
bool operator!=(const queue<T, Container>& lhs, const queue<T, Container>& rhs)
{
  return !(lhs == rhs);
}

template <class T, class Container>
bool operator<(const queue<T, Container>& lhs, const queue<T, Container>& rhs)
{
  return lhs < rhs;
}

template <class T, class Container>
bool operator>(const queue<T, Container>& lhs, const queue<T, Container>& rhs)
{
  return rhs < lhs;
}

template <class T, class Container>
bool operator<=(const queue<T, Container>& lhs, const queue<T, Container>& rhs)
{
  return !(rhs < lhs);
}

template <class T, class Container>
bool operator>=(const queue<T, Container>& lhs, const queue<T, Container>& rhs)
{
  return !(lhs < rhs);
}

// 重载 mystl 的 swap
template <class T, class Container>
void swap(queue<T, Container>& lhs, queue<T, Container>& rhs) noexcept(noexcept(lhs.swap(rhs)))
{
  lhs.swap(rhs);
}

} //namespace mystl

#endif