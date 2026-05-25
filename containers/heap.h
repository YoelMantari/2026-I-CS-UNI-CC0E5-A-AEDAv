#ifndef __HEAP_H__
#define __HEAP_H__

#include <cstddef>
#include <cctype>
#include <functional>
#include <iostream>
#include <mutex>
#include <shared_mutex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>
#include "vector.h"
#include "traits.h"
#include "../types.h"
using namespace std;

template <typename T>
struct MinHeapTrait : public BaseTrait<T, less<T>> {};

template <typename T>
struct MaxHeapTrait : public BaseTrait<T, greater<T>> {};

// clase nodo almacenado dentro del heap
template<typename Trait>
class HeapNode{
public:
    using value_type = typename Trait::value_type;
    using MySelf     = HeapNode<Trait>;

private:
    value_type m_data;
    Ref        m_ref;

public:
    HeapNode() : m_data(value_type()), m_ref(Ref()) {}
    HeapNode(value_type data, Ref ref) : m_data(data), m_ref(ref) {}

    // Devuelve una copia del valor.
    value_type getData() const { return m_data; }
    value_type& getDataRef() { return m_data; }
    const value_type& getDataRef() const { return m_data; }
    Ref getRef() const { return m_ref; }
    void setData(value_type data) { m_data = data; }
    void setRef(Ref ref) { m_ref = ref; }
};

template<typename Trait>
ostream& operator<<(ostream& os, const HeapNode<Trait>& node){
    return os << "(" << node.getData() << ", " << node.getRef() << ")";
}

template<typename Trait>
istream& operator>>(istream& is, HeapNode<Trait>& node){
    typename HeapNode<Trait>::value_type value;
    Ref ref;
    if(is >> value >> ref){
        node.setData(value);
        node.setRef(ref);
    }
    return is;
}

//class auxiliar para guardar heapnode dentro de vector
template <typename T>
struct HeapStorageTrait{
    using value_type = T;
    using Node       = VectorNode<T>;
};

// class heap 
template<typename Trait>
class Heap{
public:
    using value_type = typename Trait::value_type;
    using Comp       = typename Trait::Comp;
    using MySelf     = Heap<Trait>;
    using Node       = HeapNode<Trait>;
    using Storage    = Vector<HeapStorageTrait<Node>>;

private:
    // arreglo dinamico que representa el heap.
    Storage m_vec;
    Comp    m_comp;
    mutable shared_mutex m_mtx;

    void heapifyUpUnsafe(size_t index);
    void heapifyDownUnsafe(size_t index);
    string treeToStringUnsafe() const;

public:

    Heap() : m_vec(), m_comp() {}
    Heap(const Heap& other);
    Heap(Heap&& other);

    Heap& operator=(const Heap& other);
    Heap& operator=(Heap&& other);
    ~Heap() {}

    // inserta valor y restaura la propiedad del heap.
    void insert(value_type value, Ref ref);

    // extrae y devuelve la raiz del heap
    Node extract();

    // devlver la raiz sin removerla
    Node peek() const;
    bool isEmpty() const;
    size_t size() const;
    string toString() const;

    //foreach
    template<typename Func, typename... Args>
    void forEach(Func func, Args&&... args) const{
        std::vector<Node> snapshot;
        {
            shared_lock<shared_mutex> lock(m_mtx);
            snapshot.reserve(m_vec.size());
            for(size_t i = 0; i < m_vec.size(); ++i)
                snapshot.push_back(m_vec.at(i));
        }

        for(const auto& node : snapshot)
            func(node, std::forward<Args>(args)...);
    }
};

// constructor por copia
template<typename Trait>
Heap<Trait>::Heap(const Heap& other) : m_vec(), m_comp(){
    shared_lock<shared_mutex> lock(other.m_mtx);
    m_vec = other.m_vec;
    m_comp = other.m_comp;
}

// constructor por mov
template<typename Trait>
Heap<Trait>::Heap(Heap&& other) : m_vec(), m_comp(){
    unique_lock<shared_mutex> lock(other.m_mtx);
    m_vec = other.m_vec;
    m_comp = std::move(other.m_comp);
    other.m_vec = Storage();
    other.m_comp = Comp();
}

// asignacion por copia 
template<typename Trait>
Heap<Trait>& Heap<Trait>::operator=(const Heap& other){
    if(this != &other){
        Storage copy_vec;
        Comp copy_comp;
        {
            shared_lock<shared_mutex> other_lock(other.m_mtx);
            copy_vec = other.m_vec;
            copy_comp = other.m_comp;
        }

        unique_lock<shared_mutex> lock(m_mtx);
        m_vec = copy_vec;
        m_comp = copy_comp;
    }
    return *this;
}

// asignacion por mov
template<typename Trait>
Heap<Trait>& Heap<Trait>::operator=(Heap&& other){
    if(this != &other){
        Storage moved_vec;
        Comp moved_comp;
        {
            unique_lock<shared_mutex> other_lock(other.m_mtx);
            moved_vec = other.m_vec;
            moved_comp = std::move(other.m_comp);
            other.m_vec = Storage();
            other.m_comp = Comp();
        }

        unique_lock<shared_mutex> lock(m_mtx);
        m_vec = moved_vec;
        m_comp = std::move(moved_comp);
    }
    return *this;
}

// sube un nodo mientras tenga mas prioridad que su padre
template<typename Trait>
void Heap<Trait>::heapifyUpUnsafe(size_t index){
    while(index > 0){
        size_t parent = (index - 1) / 2;
        if(!m_comp(m_vec.at(index).getData(), m_vec.at(parent).getData()))
            break;
        m_vec.swapAt(index, parent);
        index = parent;
    }
}

// baja un nodo hasta que sus hijos no tengan mayor prioridad
template<typename Trait>
void Heap<Trait>::heapifyDownUnsafe(size_t index){
    size_t count = m_vec.size();

    while(true){
        size_t selected = index;
        size_t left = 2 * index + 1;
        size_t right = 2 * index + 2;

        if(left < count && m_comp(m_vec.at(left).getData(), m_vec.at(selected).getData()))
            selected = left;

        if(right < count && m_comp(m_vec.at(right).getData(), m_vec.at(selected).getData()))
            selected = right;

        if(selected == index)
            break;

        m_vec.swapAt(index, selected);
        index = selected;
    }
}

template<typename Trait>
string Heap<Trait>::treeToStringUnsafe() const{
    if(m_vec.empty())
        return "  (vacio)\n";

    ostringstream oss;
    size_t level_start = 0;
    size_t level_size = 1;
    size_t count = m_vec.size();

    while(level_start < count){
        oss << "  ";
        size_t end = level_start + level_size;
        if(end > count)
            end = count;

        for(size_t i = level_start; i < end; ++i)
            oss << m_vec.at(i).getData() << " ";

        oss << "\n";
        level_start += level_size;
        level_size *= 2;
    }

    return oss.str();
}

template<typename Trait>
void Heap<Trait>::insert(value_type value, Ref ref){
    unique_lock<shared_mutex> lock(m_mtx);
    m_vec.push_back(Node(value, ref), ref);
    heapifyUpUnsafe(m_vec.size() - 1);
}

template<typename Trait>
HeapNode<Trait> Heap<Trait>::extract(){
    unique_lock<shared_mutex> lock(m_mtx);
    if(m_vec.empty())
        throw out_of_range("heap esta vacio");

    Node result = m_vec.at(0);

    if(m_vec.size() == 1){
        m_vec.pop_back();
        return result;
    }

    Node last = m_vec.pop_back().getData();
    m_vec.setAt(0, last, last.getRef());
    heapifyDownUnsafe(0);
    return result;
}

// cnsultar heap
template<typename Trait>
HeapNode<Trait> Heap<Trait>::peek() const{
    shared_lock<shared_mutex> lock(m_mtx);
    if(m_vec.empty())
        throw out_of_range("heap vacio");
    return m_vec.at(0);
}

template<typename Trait>
bool Heap<Trait>::isEmpty() const{
    shared_lock<shared_mutex> lock(m_mtx);
    return m_vec.empty();
}

template<typename Trait>
size_t Heap<Trait>::size() const{
    shared_lock<shared_mutex> lock(m_mtx);
    return m_vec.size();
}

template<typename Trait>
string Heap<Trait>::toString() const{
    shared_lock<shared_mutex> lock(m_mtx);
    ostringstream oss;
    oss << "heap: [";
    for(size_t i = 0; i < m_vec.size(); ++i){
        if(i > 0)
            oss << ",";
        oss << m_vec.at(i);
    }
    oss << "]\nTree:\n" << treeToStringUnsafe();
    return oss.str();
}

//operator<<
template<typename Trait>
ostream& operator<<(ostream& os, const Heap<Trait>& heap){
    return os << heap.toString();
}

//operator>>
template<typename Trait>
istream& operator>>(istream& is, Heap<Trait>& heap){
    typename Heap<Trait>::value_type value;
    Ref ref;

    is >> ws;
    int next = is.peek();
    if(next == EOF)
        return is;

    if(next != '[' && next != '(' && !isdigit(next) && next != '-'){
        char ignored;
        while(is.get(ignored) && ignored != '[') {}
        if(!is){
            is.setstate(ios_base::failbit);
            return is;
        }
        is.unget();
    }

    is >> ws;
    if(is.peek() != '['){
        while(is >> value >> ref)
            heap.insert(value, ref);
        return is;
    }

    char ch;
    is >> ch;
    if(ch != '['){
        is.setstate(ios_base::failbit);
        return is;
    }

    is >> ws;
    if(is.peek() == ']'){
        is.get();
        return is;
    }

    while(is){
        char open, comma, close;
        if(!(is >> open) || open != '('){
            is.setstate(ios_base::failbit);
            return is;
        }

        if(!(is >> value >> comma >> ref >> close) || comma != ',' || close != ')'){
            is.setstate(ios_base::failbit);
            return is;
        }

        heap.insert(value, ref);

        is >> ws;
        ch = static_cast<char>(is.peek());
        if(ch == ','){
            is.get();
            continue;
        }
        if(ch == ']'){
            is.get();
            return is;
        }

        is.setstate(ios_base::failbit);
        return is;
    }

    return is;
}

#endif // __HEAP_H__
