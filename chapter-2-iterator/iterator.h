#ifndef __MYSTL_ITERATOR_H__
#define __MYSTL_ITERATOR_H__

//迭代器的模板 

#include <cstddef>
#include "type_traits.h"

namespace mystl
{

// 五种迭代器
// only read
struct input_iterator_tag {};
// only write
struct output_iterator_tag {};
// only forward
struct forward_iterator_tag : public input_iterator_tag {};
// bidirection
struct bidirectional_iterator_tag : forward_iterator_tag {};
// random access
struct randomaccess_iterator_tag : bidirectional_iterator_tag {};

// template of iterator
template<class Type, class T, class Distance = ptrdiff_t, 
    class Pointer = T*, class Reference = T&>
    struct iterator
{
    typedef Type        iterator_type;
    typedef T           value_type;
    typedef Distance    difference_type;
    typedef Pointer     pointer;
    typedef Reference   reference;     
};

// iterator traits

// 是否有迭代去类型，通过函数偏特化来做
template<class T>
struct has_iterator_type
{
private:
    struct two
    {
        char a;
        char b;
    };
    template<class U> static two test(...);
    template<class U> static char test(typename U::iterator_type* = 0);

public:
    static const bool value = sizeof(test<T>(0)) == sizeof(char); 
};

template<class Iterator, bool>
struct iterator_traits_iml {};

template<class Iterator>
struct iterator_traits_iml<Iterator, true>
{
    typedef typename Iterator::iterator_type     iterator_type;
    typedef typename Iterator::value_type        value_type;
    typedef typename Iterator::pointer           pointer;
    typedef typename Iterator::reference         reference;
    typedef typename Iterator::difference_type   difference_type;
};

template<class Iterator, bool>
struct iterator_traits_helper {};

template<class Iterator>
struct iterator_traits_helper<Iterator, true>
    :public iterator_traits_iml<Iterator, 
    std::is_convertible<typename Iterator::iterator_type, input_iterator_tag> || 
    std::is_convertible<typename Iterator::iterator_type, output_iterator_tag>>
{};

template<class Iterator>
struct iterator_traits
    : public iterator_traits_helper<Iterator, has_iterator_type<Iterator>::value>
    {};
// iterator_traits 只接受一个迭代器，但需要判断这个迭代器是否是定义好的迭代器，
// 这就许哟啊有一个父类有一个偏特化版本是真的。从而引出helper的类型。又因为要判断迭代器类型是否是上述定义的五种。
// 从而引入iml。也可以用逻辑判断。但是偏特化好像更厉害点。（大神不需要这些解释^~^）

}


#endif