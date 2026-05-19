#ifndef __LINKEDLIST_H__
#define __LINKEDLIST_H__

#include <iostream>
#include <cstddef>
#include <string>
#include <sstream>
#include <stdexcept>
#include <mutex>
#include <shared_mutex> 
#include <utility>
#include <tuple>
#include <type_traits>
#include "general_iterator.h"
#include "util.h"
#include "../types.h"
#include "traits.h"

using namespace std;

template <typename T, typename DerivedNode = void>
class LLNode {
public:
    using value_type = T;
    using Node = std::conditional_t<std::is_void_v<DerivedNode>, LLNode, DerivedNode>;
protected:
    T   m_data;
    Ref m_ref;
    Node *m_next;
public:
    LLNode() : m_data(T()), m_ref(Ref()), m_next(nullptr) {}
    LLNode(T data, Ref ref, Node *next = nullptr)
        : m_data(data), m_ref(ref), m_next(next) {}
    virtual ~LLNode() {}

    T      getData() const { return m_data; }
    T&     getDataRef()    { return m_data; }
    void   setData(T data) { m_data = data; }
    Ref    getRef() const  { return m_ref; }
    void   setRef(Ref ref) { m_ref = ref; }
    Node*  getNext() const { return m_next; }
    Node*& getNextRef()    { return m_next; }
    void   setNext(Node *next) { m_next = next; }
};

template <typename T>
struct AscendingLinkedListTrait : BaseTrait<T, less<T>, LLNode<T>> {};

template <typename T>
struct DescendingLinkedListTrait : BaseTrait<T, greater<T>, LLNode<T>> {};

template <typename Container>
class LinkedListForwardIterator
    : public general_iterator<Container, LinkedListForwardIterator<Container>> {
public:
    using MySelf = LinkedListForwardIterator<Container>;
    using Parent = general_iterator<Container, MySelf>;
    using Parent::Parent;
};

template <typename Trait>
class LinkedList {
public:
    using value_type = typename Trait::value_type;
    using Node       = typename Trait::Node;
    using Comp       = typename Trait::Comp;
    using MySelf     = LinkedList<Trait>;

    using forward_iterator = LinkedListForwardIterator<MySelf>;
    friend forward_iterator;

protected:
    Node *m_pRoot = nullptr;
    Node *m_tail = nullptr;
    size_t m_size = 0;
    Comp   m_comp;
    mutable shared_mutex m_mtx;

    virtual void internal_insert(Node* &pPrev, const value_type &value, Ref ref) {
        if (!pPrev || m_comp(value, pPrev->getDataRef())) {
            Node *newNode = new Node(value, ref, pPrev);
            pPrev = newNode;
            m_size++;
            if (newNode->getNext() == nullptr)
                m_tail = newNode;
            return;
        }
        internal_insert(pPrev->getNextRef(), value, ref);
    }
protected:

    void copy_circular_nodes(const LinkedList &other) {
        Node *curr = other.m_pRoot;
        if (!curr) return;
        do {
            this->push_back(curr->getData(), curr->getRef());  // virtual, versión circular
            curr = curr->getNext();
        } while (curr != other.m_pRoot);
    }

public:
    LinkedList() {}

    // copia constructor
    LinkedList(const LinkedList &other)
        : m_pRoot(nullptr), m_tail(nullptr), m_size(0) {
        shared_lock<shared_mutex> lock(other.m_mtx);
        Node* curr = other.m_pRoot;
        for (size_t i = 0; i < other.m_size; ++i) {
            push_back(curr->getData(), curr->getRef());
            curr = curr->getNext();
        }
    }

    // move constructor
    LinkedList(LinkedList &&other)
        : m_pRoot(nullptr), m_tail(nullptr), m_size(0) {
        unique_lock<shared_mutex> lock(other.m_mtx);
        m_pRoot = std::exchange(other.m_pRoot, nullptr);
        m_tail  = std::exchange(other.m_tail, nullptr);
        m_size  = std::exchange(other.m_size, 0);
    }

    // Copy assignment
    LinkedList& operator=(const LinkedList &other) {
        if (this != &other) {
            clear();
            shared_lock<shared_mutex> lock(other.m_mtx);
            Node* curr = other.m_pRoot;
            for (size_t i = 0; i < other.m_size; ++i) {
                push_back(curr->getData(), curr->getRef());
                curr = curr->getNext();
            }
        }
        return *this;
    }

    // Move assignment
    LinkedList& operator=(LinkedList &&other) {
        if (this != &other) {
            clear();
            unique_lock<shared_mutex> lock(other.m_mtx);
            m_pRoot = std::exchange(other.m_pRoot, nullptr);
            m_tail  = std::exchange(other.m_tail, nullptr);
            m_size  = std::exchange(other.m_size, 0);
        }
        return *this;
    }

    virtual ~LinkedList() { clear(); }

    virtual void clear() {
        unique_lock<shared_mutex> lock(m_mtx);
        Node* current = m_pRoot;
        for (size_t i = 0; i < m_size; ++i) {
            Node* next = current->getNext();
            delete current;
            current = next;
        }
        m_pRoot = nullptr;
        m_tail = nullptr;
        m_size = 0;
    }

    virtual void push_front(value_type value, Ref ref) {
        insert(value, ref);
    }

    virtual void push_back(value_type value, Ref ref) {
        insert(value, ref);
    }

    virtual void insert(const value_type &value, Ref ref) {
        unique_lock<shared_mutex> lock(m_mtx);
        internal_insert(m_pRoot, value, ref);
        if (m_size == 1)
            m_tail = m_pRoot;
    }

    virtual std::tuple<value_type, Ref> pop_front() {
        unique_lock<shared_mutex> lock(m_mtx);
        if (!m_pRoot) throw runtime_error("Lista vacia");
        Node *temp   = m_pRoot;
        auto  result = std::make_tuple(temp->getData(), temp->getRef());
        m_pRoot = m_pRoot->getNext();
        delete temp;
        m_size--;
        if (m_size == 0) m_tail = nullptr;
        return result;
    }

    virtual std::tuple<value_type, Ref> pop_back() {
        unique_lock<shared_mutex> lock(m_mtx);
        if (!m_pRoot) throw runtime_error("Lista vacia");
        if (m_size == 1) {
            auto result = std::make_tuple(m_pRoot->getData(), m_pRoot->getRef());
            delete m_pRoot;
            m_pRoot = m_tail = nullptr;
            m_size = 0;
            return result;
        }
        Node *act = m_pRoot;
        while (act->getNext() != m_tail)
            act = act->getNext();
        auto result = std::make_tuple(m_tail->getData(), m_tail->getRef());
        delete m_tail;
        m_tail = act;
        m_tail->setNext(nullptr);
        m_size--;
        return result;
    }

    virtual value_type& operator[](size_t index) {
        shared_lock<shared_mutex> lock(m_mtx);
        if (index >= m_size) throw out_of_range("Indice fuera de rango");
        Node *act = m_pRoot;
        for (size_t i = 0; i < index; ++i)
            act = act->getNext();
        return act->getDataRef();
    }

    virtual size_t size() const {
        shared_lock<shared_mutex> lock(m_mtx);
        return m_size;
    }

    forward_iterator begin() { return forward_iterator(this, m_pRoot); }
    forward_iterator end()   { return forward_iterator(this, nullptr); }

    template <typename Func, typename... Args>
    void ForEach(Func func, Args &&...args) {
        unique_lock<shared_mutex> lock(m_mtx);
        if (m_size == 0) return;
        for (auto &item : *this)
            func(item, std::forward<Args>(args)...);
    }

    virtual void print(ostream& os) const {
        shared_lock<shared_mutex> lock(m_mtx);
        os << "[";
        Node* act = m_pRoot;
        for (size_t i = 0; i < m_size; ++i) {
            os << "(" << act->getData() << "," << act->getRef() << ")";
            if (i + 1 < m_size) os << ",";
            act = act->getNext();
        }
        os << "]";
    }

    virtual void read(istream& is) {
        char ch;
        if (!(is >> ch) || ch != '[') {
            is.clear(ios_base::failbit);
            return;
        }
        value_type val;
        Ref ref;
        char comma, parenClose;
        while (is >> ch && ch != ']') {
            if (ch == '(') {
                if (is >> val >> comma >> ref >> parenClose) {
                    if (comma == ',' && parenClose == ')')
                        insert(val, ref);
                }
            }
        }
    }

    friend ostream& operator<<(ostream& os, const LinkedList& list) {
        list.print(os);
        return os;
    }

    friend istream& operator>>(istream& is, LinkedList& list) {
        list.read(is);
        return is;
    }
};

#endif // __LINKEDLIST_H__