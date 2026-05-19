//containers/circularlinkedlist.h
#ifndef __CIRCULARLINKEDLIST_H__
#define __CIRCULARLINKEDLIST_H__

#include "linkedlist.h"

template <typename T>
struct AscendingCLLTrait : BaseTrait<T, less<T>, LLNode<T>> {};

template <typename T>
struct DescendingCLLTrait : BaseTrait<T, greater<T>, LLNode<T>> {};

template <typename Container>
class CLLForwardIterator
    : public circular_iterator<Container, CLLForwardIterator<Container>> {
public:
    using MySelf = CLLForwardIterator<Container>;
    using Parent = circular_iterator<Container, MySelf>;
    using Node   = typename Container::Node;
    CLLForwardIterator(Container *c, Node *node, Node *root)
        : Parent(c, node, root) {}
};

template <typename Trait>
class CircularLinkedList : public LinkedList<Trait> {
public:
    using value_type       = typename Trait::value_type;
    using Node             = typename Trait::Node;
    using MySelf           = CircularLinkedList<Trait>;
    using forward_iterator = CLLForwardIterator<MySelf>;
    friend forward_iterator;

    CircularLinkedList() : LinkedList<Trait>() {}

// Copy constructor
CircularLinkedList(const CircularLinkedList &other) : LinkedList<Trait>() {
    shared_lock<shared_mutex> lock(other.m_mtx);
    if (!other.m_pRoot) return;
    Node *curr = other.m_pRoot;
    do {
        this->push_back(curr->getData(), curr->getRef());
        curr = curr->getNext();
    } while (curr != other.m_pRoot);
}

    // Move constructor
    CircularLinkedList(CircularLinkedList &&other) noexcept : LinkedList<Trait>(std::move(other)) {}

    // Copy assignment
    CircularLinkedList &operator=(const CircularLinkedList &other) {
        if (this != &other) {
            this->clear();
            shared_lock<shared_mutex> lock(other.m_mtx);
            Node *curr = other.m_pRoot;
            if (curr) {
                do {
                    this->push_back(curr->getData(), curr->getRef());
                    curr = curr->getNext();
                } while (curr != other.m_pRoot);
            }
        }
        return *this;
    }

    // Move assignment
    CircularLinkedList &operator=(CircularLinkedList &&other) noexcept {
        if (this != &other) {
            this->clear();
            LinkedList<Trait>::operator=(std::move(other));
        }
        return *this;
    }

    forward_iterator cbegin() const {
        return forward_iterator(const_cast<MySelf*>(this), this->m_pRoot,
                                this->m_pRoot);
    }
    forward_iterator cend() const {
        return forward_iterator(const_cast<MySelf*>(this), nullptr,
                                this->m_pRoot);
    }
    forward_iterator begin() { return cbegin(); }
    forward_iterator end()   { return cend(); }

    void push_back(value_type value, Ref ref) override {
        unique_lock<shared_mutex> lock(this->m_mtx);
        Node *newNode = new Node(value, ref);
        if (this->m_size == 0) {
            this->m_pRoot = this->m_tail = newNode;
            newNode->setNext(newNode);
        } else {
            newNode->setNext(this->m_pRoot);
            this->m_tail->setNext(newNode);
            this->m_tail = newNode;
        }
        this->m_size++;
    }

    void push_front(value_type value, Ref ref) override {
        unique_lock<shared_mutex> lock(this->m_mtx);
        Node *newNode = new Node(value, ref);
        if (this->m_size == 0) {
            this->m_pRoot = this->m_tail = newNode;
            newNode->setNext(newNode);
        } else {
            newNode->setNext(this->m_pRoot);
            this->m_tail->setNext(newNode);
            this->m_pRoot = newNode;
        }
        this->m_size++;
    }

    tuple<value_type, Ref> pop_front() override {
        unique_lock<shared_mutex> lock(this->m_mtx);
        if (!this->m_pRoot) throw runtime_error("Lista vacia");
        Node *temp   = this->m_pRoot;
        auto  result = make_tuple(temp->getData(), temp->getRef());
        if (this->m_size == 1) {
            this->m_pRoot = this->m_tail = nullptr;
        } else {
            this->m_pRoot = temp->getNext();
            this->m_tail->setNext(this->m_pRoot);
        }
        delete temp;
        this->m_size--;
        return result;
    }

    tuple<value_type, Ref> pop_back() override {
        unique_lock<shared_mutex> lock(this->m_mtx);
        if (!this->m_pRoot) throw runtime_error("Lista vacia");
        auto result = make_tuple(this->m_tail->getData(), this->m_tail->getRef());
        if (this->m_size == 1) {
            delete this->m_tail;
            this->m_pRoot = this->m_tail = nullptr;
        } else {
            Node *act = this->m_pRoot;
            while (act->getNext() != this->m_tail)
                act = act->getNext();
            delete this->m_tail;
            this->m_tail = act;
            this->m_tail->setNext(this->m_pRoot);
        }
        this->m_size--;
        return result;
    }

    void insert(const value_type &value, Ref ref) override {
        unique_lock<shared_mutex> lock(this->m_mtx);
        Node *newNode = new Node(value, ref);
        if (this->m_size == 0) {
            this->m_pRoot = this->m_tail = newNode;
            newNode->setNext(newNode);
        } else if (this->m_comp(value, this->m_pRoot->getDataRef())) {
            newNode->setNext(this->m_pRoot);
            this->m_tail->setNext(newNode);
            this->m_pRoot = newNode;
        } else {
            Node *act = this->m_pRoot;
            while (act->getNext() != this->m_pRoot &&
                   !this->m_comp(value, act->getNext()->getDataRef()))
                act = act->getNext();
            newNode->setNext(act->getNext());
            act->setNext(newNode);
            if (act == this->m_tail) this->m_tail = newNode;
        }
        this->m_size++;
    }

    template <typename Func, typename... Args>
    void circularForEach(size_t vueltas, Func func, Args &&...args) {
        unique_lock<shared_mutex> lock(this->m_mtx);
        if (this->m_size == 0 || vueltas == 0) return;
        Node  *act   = this->m_pRoot;
        size_t pasos = this->m_size * vueltas;
        for (size_t i = 0; i < pasos; ++i) {
            func(act->getDataRef(), forward<Args>(args)...);
            act = act->getNext();
        }
    }
};

#endif