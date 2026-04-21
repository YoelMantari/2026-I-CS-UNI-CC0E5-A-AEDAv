#ifndef __LINKEDLIST_H__
#define __LINKEDLIST_H__

#include <iostream>
#include <cstddef> // size_t
#include <string>
#include <sstream>
#include <stdexcept>
#include <mutex>
#include <shared_mutex> // shared_mutex
#include "general_iterator.h"
#include "util.h"
#include "../types.h"
using namespace std;

// Forward iterator
template <typename Container>
class LinkedListForwardIterator : public general_iterator<Container, LinkedListForwardIterator<Container>>{
public:
    using MySelf = LinkedListForwardIterator<Container>;
    using Parent = general_iterator<Container, MySelf>;
    using Parent::Parent;
    // t4 avanza al siguiente nodo en cada incremento
    MySelf operator++() {
        if(this->m_pNode)
            this->m_pNode = this->m_pNode->getNext();
        return *this;
    }
};

// Linked List Node
template <typename T>
class LLNode{
    using Node = LLNode<T>;
private:
    T   m_data;
    Ref m_ref;
    Node *m_next;
public:
    LLNode() : m_data(T()), m_ref(Ref()), m_next(nullptr) {}
    LLNode(T data, Ref ref) : m_data(data), m_ref(ref), m_next(nullptr) {}
    LLNode(T data, Ref ref, Node *next) : m_data(data), m_ref(ref), m_next(next) {}
    virtual ~LLNode() {}

    T      getData() const { return m_data; }
    T&     getDataRef()    { return m_data; }
    void   setData(T data) { m_data = data; }
    Ref    getRef() const { return m_ref; }
    void   setRef(Ref ref) { m_ref = ref; }
    Node*  getNext() const { return m_next; }
    Node*& getNextRef()    { return m_next; }
    void   setNext(Node *next) { m_next = next; }
};

template <typename T>
ostream& operator<<(ostream& os, const LLNode<T>& node){
    return os << "(" << node.getData() << ", " << node.getRef() << ")";
}

template <typename T>
struct AscendingLinkedListTrait{
    using value_type = T;
    using Node = LLNode<T>;
    using Comp = less<T>;
};

template <typename T>
struct DescendingLinkedListTrait{
    using value_type = T;
    using Node = LLNode<T>;
    using Comp = greater<T>;
};

template <typename Trait>
class LinkedList{
public:
    using value_type = typename Trait::value_type;
    using Node       = typename Trait::Node;
    using Comp       = typename Trait::Comp;
    using MySelf     = LinkedList<Trait>;

    using forward_iterator = LinkedListForwardIterator<MySelf>;
    friend forward_iterator;

private:
    Node *m_pRoot = nullptr;
    Node *m_tail = nullptr;
    size_t m_size = 0;
    Comp   m_comp;
    // t13 sincroniza acceso concurrente de lectura y escritura
    mutable shared_mutex m_mtx;
private:
    static void free_nodes(Node *p){
        while(p){
            Node *n = p->getNext();
            delete p;
            p = n;
        }
    }

    void clear_unsafe(){
        free_nodes(m_pRoot);
        m_pRoot = nullptr;
        m_tail = nullptr;
        m_size = 0;
    }

    static void copy_chain(Node *src, Node* &dst_root, Node* &dst_tail, size_t &dst_size){
        dst_root = nullptr;
        dst_tail = nullptr;
        dst_size = 0;
        try{
            while(src){
                Node *n = new Node(src->getData(), src->getRef());
                if(!dst_root)
                    dst_root = n;
                else
                    dst_tail->setNext(n);
                dst_tail = n;
                dst_size++;
                src = src->getNext();
            }
        }catch(...){
            free_nodes(dst_root);
            dst_root = nullptr;
            dst_tail = nullptr;
            dst_size = 0;
            throw;
        }
    }
public:
    LinkedList() {}
    // t1 crea una copia profunda de todos los nodos
    LinkedList(const LinkedList &other){ // Copy constructor
        shared_lock<shared_mutex> lock(other.m_mtx);
        copy_chain(other.m_pRoot, m_pRoot, m_tail, m_size);
    }
    // t2 mueve la propiedad de nodos desde otra lista
    LinkedList(LinkedList &&other){ // Move constructor
        unique_lock<shared_mutex> lock(other.m_mtx);
        m_pRoot = other.m_pRoot;
        m_tail = other.m_tail;
        m_size = other.m_size;
        other.m_pRoot = nullptr;
        other.m_tail = nullptr;
        other.m_size = 0;
    }
    // t3 libera todos los nodos de forma segura al destruir
    virtual        ~LinkedList() {
        unique_lock<shared_mutex> lock(m_mtx);
        clear_unsafe();
    }
    // t5 inserta un elemento al inicio
    virtual void    push_front(value_type value, Ref ref);
    // t6 elimina el primer elemento si existe
    virtual void    pop_front();
    // t7 inserta un elemento al final
    virtual void    push_back(value_type value, Ref ref);
    // t8 elimina el ultimo elemento si existe
    virtual void    pop_back();
private:
            void    internal_insert(Node* &pPrev, const value_type &value, Ref ref);
public:
    virtual void    insert(const value_type &value, Ref ref);
    
    // t9 devuelve referencia al elemento por indice
    virtual value_type& operator[](size_t index);
    virtual size_t  size() const;
    virtual string  toString() const;

    forward_iterator begin() { return forward_iterator(this, m_pRoot); }
    forward_iterator end()   { return forward_iterator(this, nullptr); }

    // t12 recorre en orden y aplica una funcion a cada elemento
    template <typename Func, typename... Args>
    void ForEach(Func func, Args &&...  args){
        unique_lock<shared_mutex> lock(m_mtx);
        ::ForEach(begin(), end(), func, std::forward<Args>(args)... );
    }
};

template <typename Trait>
void LinkedList<Trait>::internal_insert(Node* &pPrev, const value_type &value, Ref ref){
    if(!pPrev || m_comp(value, pPrev->getDataRef())){
        pPrev = new Node(value, ref, pPrev);
        m_size++;
        if(pPrev == m_pRoot)
            m_tail = pPrev;
        return;
    }
    internal_insert(pPrev->getNextRef(), value, ref);
}

template <typename Trait>
void LinkedList<Trait>::insert(const value_type &value, Ref ref){
    unique_lock<shared_mutex> lock(m_mtx);
    internal_insert(m_pRoot, value, ref);
    m_tail = m_pRoot;
    while(m_tail && m_tail->getNext())
        m_tail = m_tail->getNext();
}

// t5 inserta un elemento al inicio
template <typename Trait>
void LinkedList<Trait>::push_front(value_type value, Ref ref){
    unique_lock<shared_mutex> lock(m_mtx);
    Node *n = new Node(value, ref, m_pRoot);
    m_pRoot = n;
    if(!m_tail)
        m_tail = n;
    m_size++;
}

// t6 elimina el primer elemento si existe
template <typename Trait>
void LinkedList<Trait>::pop_front(){
    unique_lock<shared_mutex> lock(m_mtx);
    if(!m_pRoot)
        return;
    Node *n = m_pRoot;
    m_pRoot = m_pRoot->getNext();
    delete n;
    m_size--;
    if(m_size == 0)
        m_tail = nullptr;
}

// t7 inserta un elemento al final
template <typename Trait>
void LinkedList<Trait>::push_back(value_type value, Ref ref){
    unique_lock<shared_mutex> lock(m_mtx);
    Node *n = new Node(value, ref);
    if(!m_pRoot){
        m_pRoot = n;
        m_tail = n;
    }else{
        m_tail->setNext(n);
        m_tail = n;
    }
    m_size++;
}

// t8 elimina el ultimo elemento si existe
template <typename Trait>
void LinkedList<Trait>::pop_back(){
    unique_lock<shared_mutex> lock(m_mtx);
    if(!m_pRoot)
        return;
    if(m_pRoot == m_tail){
        delete m_pRoot;
        m_pRoot = nullptr;
        m_tail = nullptr;
        m_size = 0;
        return;
    }
    Node *p = m_pRoot;
    while(p->getNext() != m_tail)
        p = p->getNext();
    delete m_tail;
    m_tail = p;
    m_tail->setNext(nullptr);
    m_size--;
}

template <typename Trait>
typename LinkedList<Trait>::value_type& LinkedList<Trait>::operator[](size_t index){
    shared_lock<shared_mutex> lock(m_mtx);
    if(index >= m_size)
        throw out_of_range("index out of range");
    Node *p = m_pRoot;
    for(size_t i = 0; i < index; ++i)
        p = p->getNext();
    return p->getDataRef();
}

template <typename Trait>
size_t LinkedList<Trait>::size() const{
    shared_lock<shared_mutex> lock(m_mtx);
    return m_size;
}

template <typename Trait>
string LinkedList<Trait>::toString() const{
    shared_lock<shared_mutex> lock(m_mtx);
    ostringstream oss;
    oss << "[";
    Node *p = m_pRoot;
    size_t i = 0;
    while(p){
        if(i > 0)
            oss << ",";
        oss << *p;
        p = p->getNext();
        i++;
    }
    oss << "]";
    return oss.str();
}

template <typename Trait>
// t11 escribe la lista en formato legible de salida
ostream& operator<<(ostream& os, const LinkedList<Trait>& list){
    return os << list.toString();
}

template <typename Trait>
// t10 lee n pares valor ref y construye la lista
istream& operator>>(istream& is, LinkedList<Trait>& list){
    size_t n = 0;
    if(!(is >> n))
        return is;
    while(list.size() > 0)
        list.pop_front();
    for(size_t i = 0; i < n; ++i){
        typename LinkedList<Trait>::value_type v;
        Ref r;
        if(!(is >> v >> r))
            break;
        list.push_back(v, r);
    }
    return is;
}



#endif // __LINKEDLIST_H__