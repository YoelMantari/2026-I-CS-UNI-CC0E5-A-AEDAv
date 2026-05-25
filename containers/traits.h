#ifndef __TRAITS_H__
#define __TRAITS_H__

#include <functional>

template <typename T, typename _Comp = std::less<T>>
struct BaseTrait{
    using value_type = T;
    using Comp       = _Comp;
};

template <typename T>
struct AscendingTrait : public BaseTrait<T, std::less<T>>{
};

template <typename T>
struct DescendingTrait : public BaseTrait<T, std::greater<T>>{
};

#endif // __TRAITS_H__
