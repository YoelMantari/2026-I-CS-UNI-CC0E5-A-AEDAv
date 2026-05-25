#ifndef __UTIL_H__
#define __UTIL_H__
#include <ostream>
#include <string>
#include <utility>

template <typename Container>
void Print(Container& c, std::ostream &os){
    os << c << std::endl;
}

template <typename T>
void PrintX(T& elem, std::ostream &os, std::string sep){
    os << elem << sep;
}

template <typename Container, typename Func>
void ForEach(Container& c, Func func){
    for(auto it = c.begin(); it != c.end(); ++it)
        func(*it);
}

template <typename Iterator, typename Func, typename... Args>
void ForEach(Iterator begin, Iterator end, Func func, Args&&... args){
    for(auto it = begin; it != end; ++it)
        func(*it, std::forward<Args>(args)...);
}

template <typename Container, typename Func, typename... Args>
void ForEach(Container& container, Func func, Args&&... args){
    ForEach(container.begin(), container.end(),
            func, std::forward<Args>(args)...);
}
#endif