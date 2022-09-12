#include "allocator.h"
#include "alloc.h"
#include "uninitialized.h"
#include <iostream>
#include <memory>

class test_object
{
public:
    int a;
public:
    test_object()
    {
        std::cout << "test_object()" << std::endl;
        a = 111;
    }
    test_object(int value) : a(value) {
        std::cout << "test_object(int value)" << std::endl;
    }
    test_object(const test_object& value) : a(value.a) 
    {
        std::cout << "test_object(const test_object& value)" << std::endl;
    }
    test_object(test_object&& value) : a(value.a)    //测试右值再调用右值
    {
        std::cout << "test_object(test_object&& value)" << std::endl;
    }
    void operator=(const test_object& value)
    {
        std::cout << "test_object=" << std::endl;
        a = value.a;
    }

    void prints()
    {
        std::cout << "test_object.a: " << a << std::endl;
    }
}; 

void test_allocator()
{
    test_object obj1(2);
    test_object obj2(120);

    mystl::allocator<test_object> alloc;
    auto ptr = alloc.allocate(2);
    alloc.construct(ptr, obj1);
    alloc.construct(ptr+1, std::move(obj2));

    std::cout << &(obj2.a) << std::endl;
    std::cout << &((*(ptr+1)).a) << std::endl;

    ptr->prints();
    (ptr+1)->prints();
}

void test_alloc_uninitialized()
{
    mystl::simple_alloc<int, mystl::default_alloc> alloc;
    int* ptr = (int*)alloc.allocate(2);
    int array[2] = {1, 2};

    new(ptr) int(100);
    new(ptr + 1) int(120);
    std::cout << *ptr << std::endl;
    std::cout << *(ptr+1) << std::endl << std::endl;

    mystl::uninitialized_copy(array, array + 2, ptr);
    std::cout << *ptr << std::endl;
    std::cout << *(ptr+1) << std::endl << std::endl;

    mystl::uninitialized_fill(ptr, ptr + 2, 200);
    std::cout << *ptr << std::endl;
    std::cout << *(ptr+1) << std::endl << std::endl;

    mystl::uninitialized_fill_n(ptr, 2, 300);
    std::cout << *ptr << std::endl;
    std::cout << *(ptr+1) << std::endl << std::endl;

}

void test_alloc_ctr()
{
    mystl::simple_alloc<test_object, mystl::malloc_alloc> alloc;
  
    auto ptr = alloc.allocate(2);
    *ptr = 120;

    std::cout << ptr->a << std::endl;


}

int main()
{
    test_alloc_ctr();
}