//containers/circulardoublelinkedlist.h
#ifndef __CIRCULARDOUBLELINKEDLIST_H__
#define __CIRCULARDOUBLELINKEDLIST_H__

#include "doublelinkedlist.h"
#include "circularlinkedlist.h"


// Traits para CircularDoubleLinkedList
template <typename T>
struct AscendingCDLLTrait : BaseTrait<T, less<T>, DLLNode<T>> {};

template <typename T>
struct DescendingCDLLTrait : BaseTrait<T, greater<T>, DLLNode<T>> {};

template <typename Container>
class CDLLForwardIterator
    : public circular_iterator<Container, CDLLForwardIterator<Container>> {
public:
    using MySelf = CDLLForwardIterator<Container>;
    using Parent = circular_iterator<Container, MySelf>;
    using Node   = typename Container::Node;
    CDLLForwardIterator(Container *c, Node *node, Node *root)
        : Parent(c, node, root) {}
};

template <typename Container>
class CDLLBackwardIterator
    : public circular_iterator<Container, CDLLBackwardIterator<Container>> {
public:
    using MySelf = CDLLBackwardIterator<Container>;
    using Parent = circular_iterator<Container, MySelf>;
    using Node   = typename Container::Node;
    CDLLBackwardIterator(Container *c, Node *node, Node *root)
        : Parent(c, node, root) {}

    MySelf &operator++() {
        if (this->m_pNode) {
            Node *prev    = this->m_pNode->getPrev();
            this->m_pNode = (prev == this->m_pRoot) ? nullptr : prev;
        }
        return *this;
    }
};

template <typename Trait>
class CircularDoubleLinkedList : public DoubleLinkedList<Trait> {
public:
    using value_type        = typename Trait::value_type;
    using Node              = typename Trait::Node;
    using MySelf            = CircularDoubleLinkedList<Trait>;
    using forward_iterator  = CDLLForwardIterator<MySelf>;
    using backward_iterator = CDLLBackwardIterator<MySelf>;
    friend forward_iterator;
    friend backward_iterator;

    CircularDoubleLinkedList() : DoubleLinkedList<Trait>() {}

    // Copy constructor
    CircularDoubleLinkedList(const CircularDoubleLinkedList &other) : DoubleLinkedList<Trait>() {
        shared_lock<shared_mutex> lock(other.m_mtx);
        if (!other.m_pRoot) return;
        Node *curr = other.m_pRoot;
        do {
            this->push_back(curr->getData(), curr->getRef());
            curr = curr->getNext();
        } while (curr != other.m_pRoot);
    }

    // Move constructor
    CircularDoubleLinkedList(CircularDoubleLinkedList &&other) noexcept : DoubleLinkedList<Trait>(std::move(other)) {}

    // Copy assignment
    CircularDoubleLinkedList &operator=(const CircularDoubleLinkedList &other) {
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
    CircularDoubleLinkedList &operator=(CircularDoubleLinkedList &&other) noexcept {
        if (this != &other) {
            this->clear();
            DoubleLinkedList<Trait>::operator=(std::move(other));
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
    backward_iterator crbegin() {
        return backward_iterator(this, this->m_tail, this->m_tail);
    }
    backward_iterator crend() {
        return backward_iterator(this, nullptr, this->m_tail);
    }
    forward_iterator begin() { return cbegin(); }
    forward_iterator end()   { return cend(); }
    backward_iterator rbegin() { return crbegin(); }
    backward_iterator rend()   { return crend(); }

    void push_back(value_type value, Ref ref) override {
        unique_lock<shared_mutex> lock(this->m_mtx);
        Node *newNode = new Node(value, ref);
        if (this->m_size == 0) {
            this->m_pRoot = this->m_tail = newNode;
            newNode->setNext(newNode);
            newNode->setPrev(newNode);
        } else {
            newNode->setNext(this->m_pRoot);
            newNode->setPrev(this->m_tail);
            this->m_tail->setNext(newNode);
            this->m_pRoot->setPrev(newNode);
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
            newNode->setPrev(newNode);
        } else {
            newNode->setNext(this->m_pRoot);
            newNode->setPrev(this->m_tail);
            this->m_pRoot->setPrev(newNode);
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
            this->m_pRoot->setPrev(this->m_tail);
            this->m_tail->setNext(this->m_pRoot);
        }
        delete temp;
        this->m_size--;
        return result;
    }

    tuple<value_type, Ref> pop_back() override {
        unique_lock<shared_mutex> lock(this->m_mtx);
        if (!this->m_pRoot) throw runtime_error("Lista vacia");
        Node *temp   = this->m_tail;
        auto  result = make_tuple(temp->getData(), temp->getRef());
        if (this->m_size == 1) {
            this->m_pRoot = this->m_tail = nullptr;
        } else {
            this->m_tail = temp->getPrev();
            this->m_tail->setNext(this->m_pRoot);
            this->m_pRoot->setPrev(this->m_tail);
        }
        delete temp;
        this->m_size--;
        return result;
    }

    void insert(const value_type &value, Ref ref) override {
        unique_lock<shared_mutex> lock(this->m_mtx);
        Node *newNode = new Node(value, ref);
        if (this->m_size == 0) {
            this->m_pRoot = this->m_tail = newNode;
            newNode->setNext(newNode);
            newNode->setPrev(newNode);
        } else if (this->m_comp(value, this->m_pRoot->getDataRef())) {
            newNode->setNext(this->m_pRoot);
            newNode->setPrev(this->m_tail);
            this->m_pRoot->setPrev(newNode);
            this->m_tail->setNext(newNode);
            this->m_pRoot = newNode;
        } else {
            Node *act = this->m_pRoot;
            while (act->getNext() != this->m_pRoot &&
                   !this->m_comp(value, act->getNext()->getDataRef()))
                act = act->getNext();
            Node *following = act->getNext();
            newNode->setNext(following);
            newNode->setPrev(act);
            act->setNext(newNode);
            following->setPrev(newNode);
            if (act == this->m_tail) this->m_tail = newNode;
        }
        this->m_size++;
    }

    template <typename Func, typename... Args>
    void circularForEach(size_t vueltas, auto direction, Func func, Args &&...args) {
        unique_lock<shared_mutex> lock(this->m_mtx);
        if (!this->m_pRoot || vueltas == 0) return;
        Node  *act   = (direction >= 0) ? this->m_pRoot : this->m_tail;
        size_t pasos = this->m_size * vueltas;
        for (size_t i = 0; i < pasos; ++i) {
            func(act->getDataRef(), forward<Args>(args)...);
            act = (direction >= 0) ? act->getNext() : act->getPrev();
        }
    }

    template <typename Func, typename... Args>
    void ReverseForEach(Func func, Args &&...args) {
        unique_lock<shared_mutex> lock(this->m_mtx);
        if (this->m_size == 0) return;
        for (auto it = crbegin(); it != crend(); ++it)
            func(*it, std::forward<Args>(args)...);
    }
};

#endif