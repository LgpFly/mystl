#ifndef __MYSTL_TYPE_TRAITS_H__
#define __MYSTL_TYPE_TRAITS_H__

// 用于提取类型信息

#include <type_traits>

namespace mystd
{

template<class T, T v>
struct m_int_constant
{
    static constexpr T value = v;
};

template<bool b>
using m_booler_constant = m_integral_constant<bool, b>;

typedef m_booler_constant<true> m_true_type;
typedef m_booler_constant<false> m_flase_type;



}


#endif