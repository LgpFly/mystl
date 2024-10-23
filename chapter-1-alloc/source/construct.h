#ifndef __MYSTL_CONSTRUCT_H__
#define __MYSTL_CONSTRUCT_H__

// mystl中全局的construct和destroy

#include <new>

namespace mystl
{

// 参数是一个指针
template<class T>
void construct(T *ptr)
{
    ::new ((void*)ptr) T();
}

// 参数是一个指针和值
template<class T1, class T2>
void construct(T1*ptr, const T2& value)
{
    ::new ((void*)ptr) T1(value);
}






}









#endif