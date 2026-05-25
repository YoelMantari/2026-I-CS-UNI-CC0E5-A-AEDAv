#ifndef __VECTOR_H__
#define __VECTOR_H__

#include <cstddef>
#include <iostream>
#include <mutex>
#include <shared_mutex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include "general_iterator.h"
#include "util.h"
#include "../types.h"
using namespace std;

template <typename Container>
class vector_forward_iterator : public general_iterator<Container, vector_forward_iterator<Container>> {
public:
    using MySelf = vector_forward_iterator<Container>;
    using Parent = general_iterator<Container, MySelf>;
    using Parent::Parent;
    MySelf operator++() { this->m_pNode++; return *this; }
};

template <typename Container>
class vector_backward_iterator : public general_iterator<Container, vector_backward_iterator<Container>> {
public:
    using MySelf = vector_backward_iterator<Container>;
    using Parent = general_iterator<Container, MySelf>;
    using Parent::Parent;
    MySelf operator++() { this->m_pNode--; return *this; }
};

template <typename T>
class VectorNode{
    T   m_data;
    Ref m_ref;

public:
    using value_type = T;

    VectorNode() : m_data(T()), m_ref(Ref()) {}
    VectorNode(T data, Ref ref) : m_data(data), m_ref(ref) {}
    VectorNode(const VectorNode &other) : m_data(other.m_data), m_ref(other.m_ref) {}
    VectorNode(VectorNode &&other) noexcept : m_data(move(other.m_data)), m_ref(move(other.m_ref)) {}

    VectorNode& operator=(const VectorNode &other) {
        if(this != &other){
            m_data = other.m_data;
            m_ref = other.m_ref;
        }
        return *this;
    }

    VectorNode& operator=(VectorNode &&other) noexcept {
        if(this != &other){
            m_data = move(other.m_data);
            m_ref = move(other.m_ref);
        }
        return *this;
    }

    T    getData() const { return m_data; }
    T&   getDataRef() { return m_data; }
    const T& getDataRef() const { return m_data; }
    void setData(T data) { m_data = data; }
    Ref  getRef() const { return m_ref; }
    void setRef(Ref ref) { m_ref = ref; }
};

template <typename T>
ostream& operator<<(ostream& os, const VectorNode<T>& node){
    return os << "(" << node.getData() << ", " << node.getRef() << ")";
}

template <typename T>
struct VectorTrait{
    using value_type = T;
    using Node       = VectorNode<T>;
};

template <typename T, typename = void>
struct VectorTraitAdapter : public VectorTrait<T>{
};

template <typename T>
struct VectorTraitAdapter<T, void_t<typename T::value_type, typename T::Node>>{
    using value_type = typename T::value_type;
    using Node       = typename T::Node;
};

template <typename Trait>
class Vector{
public:
    using  Adapter           = VectorTraitAdapter<Trait>;
    using  value_type        = typename Adapter::value_type;
    using  Node              = typename Adapter::Node;
    using  forward_iterator  = vector_forward_iterator<Vector<Trait>>;
    friend forward_iterator;
    using  backward_iterator = vector_backward_iterator<Vector<Trait>>;
    friend backward_iterator;

private:
    size_t  m_capacity;
    size_t  m_size;
    Node   *m_data;
    mutable shared_mutex m_mtx;

    void resizeUnsafe();

public:
    Vector(size_t capacity = 10);
    Vector(const Vector& other);
    Vector& operator=(const Vector& other);
    virtual ~Vector();

    virtual void   push_back(value_type value, Ref ref);
    virtual Node   pop_back();
    virtual size_t size() const;
    virtual bool   empty() const;
    virtual string toString() const;

    value_type operator[](size_t index) const;
    value_type at(size_t index) const;
    Node nodeAt(size_t index) const;
    Ref refAt(size_t index) const;
    void setAt(size_t index, value_type value);
    void setAt(size_t index, value_type value, Ref ref);
    void swapAt(size_t left, size_t right);

private:
    forward_iterator begin() { return forward_iterator(this, m_data); }
    forward_iterator end()   { return forward_iterator(this, m_data + m_size); }

    backward_iterator rbegin() { return backward_iterator(this, m_data + m_size - 1); }
    backward_iterator rend()   { return backward_iterator(this, m_data - 1); }

public:
    template <typename Func, typename... Args>
    void ForEach(Func func, Args &&...  args){
        Vector copy(*this);
        for(size_t i = 0; i < copy.size(); ++i)
            func(copy.at(i), std::forward<Args>(args)...);
    }

    template <typename Func, typename... Args>
    void ReverseForEach(Func func, Args &&...  args){
        Vector copy(*this);
        for(size_t i = copy.size(); i > 0; --i)
            func(copy.at(i - 1), std::forward<Args>(args)...);
    }
};

template <typename Trait>
Vector<Trait>::Vector(size_t capacity){
    m_capacity = (capacity == 0) ? 1 : capacity;
    m_size = 0;
    m_data = new Node[m_capacity];
}

template <typename Trait>
Vector<Trait>::Vector(const Vector& other){
    shared_lock<shared_mutex> lock(other.m_mtx);
    m_capacity = other.m_capacity;
    m_size     = other.m_size;
    m_data     = new Node[m_capacity];
    for(size_t i = 0; i < m_size; ++i)
        m_data[i] = other.m_data[i];
}

template <typename Trait>
Vector<Trait>& Vector<Trait>::operator=(const Vector& other){
    if(this != &other){
        shared_lock<shared_mutex> olock(other.m_mtx);
        unique_lock<shared_mutex> lock(m_mtx);
        Node *new_data = new Node[other.m_capacity];
        for(size_t i = 0; i < other.m_size; ++i)
            new_data[i] = other.m_data[i];
        delete[] m_data;
        m_capacity = other.m_capacity;
        m_size     = other.m_size;
        m_data     = new_data;
    }
    return *this;
}

template <typename Trait>
Vector<Trait>::~Vector(){
    delete [] m_data;
}

template <typename Trait>
void Vector<Trait>::resizeUnsafe(){
    m_capacity = (m_capacity < 10) ? m_capacity + 10 : m_capacity * 2;
    Node *new_data = new Node[m_capacity];
    for(size_t i = 0; i < m_size; ++i)
        new_data[i] = m_data[i];
    delete [] m_data;
    m_data = new_data;
}

template <typename Trait>
void Vector<Trait>::push_back(value_type value, Ref ref){
    unique_lock<shared_mutex> lock(m_mtx);
    if(m_size == m_capacity)
        resizeUnsafe();
    m_data[m_size++] = Node(value, ref);
}

template <typename Trait>
typename Vector<Trait>::Node Vector<Trait>::pop_back(){
    unique_lock<shared_mutex> lock(m_mtx);
    if(m_size == 0) throw out_of_range("vector vacio");
    return m_data[--m_size];
}

template <typename Trait>
size_t Vector<Trait>::size() const{
    shared_lock<shared_mutex> lock(m_mtx);
    return m_size;
}

template <typename Trait>
bool Vector<Trait>::empty() const{
    shared_lock<shared_mutex> lock(m_mtx);
    return m_size == 0;
}

template <typename Trait>
typename Vector<Trait>::value_type Vector<Trait>::operator[](size_t index) const{
    return at(index);
}

template <typename Trait>
typename Vector<Trait>::value_type Vector<Trait>::at(size_t index) const{
    shared_lock<shared_mutex> lock(m_mtx);
    if(index >= m_size) throw out_of_range("indice fuera de rango");
    return m_data[index].getData();
}

template <typename Trait>
typename Vector<Trait>::Node Vector<Trait>::nodeAt(size_t index) const{
    shared_lock<shared_mutex> lock(m_mtx);
    if(index >= m_size) throw out_of_range("indice fuera de rango");
    return m_data[index];
}

template <typename Trait>
Ref Vector<Trait>::refAt(size_t index) const{
    shared_lock<shared_mutex> lock(m_mtx);
    if(index >= m_size) throw out_of_range("indice fuera de rango");
    return m_data[index].getRef();
}

template <typename Trait>
void Vector<Trait>::setAt(size_t index, value_type value){
    unique_lock<shared_mutex> lock(m_mtx);
    if(index >= m_size) throw out_of_range("indice fuera de rango");
    m_data[index].setData(value);
}

template <typename Trait>
void Vector<Trait>::setAt(size_t index, value_type value, Ref ref){
    unique_lock<shared_mutex> lock(m_mtx);
    if(index >= m_size) throw out_of_range("indice fuera de rango");
    m_data[index].setData(value);
    m_data[index].setRef(ref);
}

template <typename Trait>
void Vector<Trait>::swapAt(size_t left, size_t right){
    unique_lock<shared_mutex> lock(m_mtx);
    if(left >= m_size || right >= m_size) throw out_of_range("indice fuera de rango");
    swap(m_data[left], m_data[right]);
}

template <typename Trait>
string Vector<Trait>::toString() const{
    shared_lock<shared_mutex> lock(m_mtx);
    ostringstream oss;
    oss << "[";
    for(size_t i = 0; i < m_size; ++i){
        if(i > 0)
            oss << ",";
        oss << m_data[i];
    }
    oss << "]";
    return oss.str();
}

template <typename Trait>
ostream& operator<<(ostream& os, const Vector<Trait>& v){
    return os << v.toString();
}

template <typename Trait>
istream& operator>>(istream& is, Vector<Trait>& v){
    typename Vector<Trait>::value_type value;
    Ref ref;
    while(is >> value >> ref)
        v.push_back(value, ref);
    return is;
}

void DemoVector();
void DemoConcurrentVector();

#endif // __VECTOR_H__
