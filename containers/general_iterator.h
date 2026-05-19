#ifndef __ITERATOR_H__
#define __ITERATOR_H__

#include <utility>

template <typename Container, class IteratorBase>
class general_iterator {
public:
    using Node = typename Container::Node;
    using myself = general_iterator<Container, IteratorBase>;

protected:
    Container* m_pContainer;
    Node* m_pNode;

public:
    general_iterator(Container* pContainer = nullptr,
                     Node* pNode = nullptr)
        : m_pContainer(pContainer),
          m_pNode(pNode) {}

    general_iterator(const myself& other)
        : m_pContainer(other.m_pContainer),
          m_pNode(other.m_pNode) {}

    general_iterator(myself&& other) noexcept
        : m_pContainer(std::move(other.m_pContainer)),
          m_pNode(std::move(other.m_pNode)) {}

    myself& operator=(const myself& other) {
        if (this != &other) {
            m_pContainer = other.m_pContainer;
            m_pNode = other.m_pNode;
        }
        return *this;
    }

    myself& operator=(myself&& other) noexcept {
        if (this != &other) {
            m_pContainer = std::move(other.m_pContainer);
            m_pNode = std::move(other.m_pNode);
        }
        return *this;
    }

    Node* getNode() const {
        return m_pNode;
    }

    friend bool operator==(const IteratorBase& a,
                           const IteratorBase& b) {
        return a.getNode() == b.getNode();
    }

    friend bool operator!=(const IteratorBase& a,
                           const IteratorBase& b) {
        return a.getNode() != b.getNode();
    }

    typename Container::value_type& operator*() {
        return m_pNode->getDataRef();
    }

    const typename Container::value_type& operator*() const {
        return m_pNode->getDataRef();
    }

    typename Container::value_type* operator->() {
        return &(m_pNode->getDataRef());
    }

    const typename Container::value_type* operator->() const {
        return &(m_pNode->getDataRef());
    }
};

template <typename Container, class IteratorBase>
class circular_iterator
    : public general_iterator<Container, IteratorBase> {
public:
    using Node = typename Container::Node;
    using Base = general_iterator<Container, IteratorBase>;

protected:
    Node* m_pRoot;

public:
    circular_iterator(Container* pContainer = nullptr,
                      Node* pNode = nullptr,
                      Node* pRoot = nullptr)
        : Base(pContainer, pNode),
          m_pRoot(pRoot) {}

    IteratorBase& operator++() {
        if (this->m_pNode) {
            Node* next = this->m_pNode->getNext();
            this->m_pNode = (next == m_pRoot)
                                ? nullptr
                                : next;
        }
        return *static_cast<IteratorBase*>(this);
    }
};

#endif