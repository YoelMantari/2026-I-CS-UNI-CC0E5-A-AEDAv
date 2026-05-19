#ifndef __BINARYTREE_H__
#define __BINARYTREE_H__

#include <cstddef>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <stack>
#include <vector>
#include <utility>
#include <mutex>
#include <shared_mutex>
#include "containers/general_iterator.h"

// uso de nodos CRTP
template <typename Derived, typename T>
class BinaryTreeNodeBase {
public:
    using value_type = T;
    using height_t = std::size_t;            // tipo semántico para altura
    T m_data;
    Derived* m_pChild[2];

    BinaryTreeNodeBase(T data) : m_data(data), m_pChild{nullptr, nullptr} {}
    virtual ~BinaryTreeNodeBase() = default;

    T& getDataRef() { return m_data; }
    T getData() const { return m_data; }

    // sobrecarga de << (impresión es responsabilidad del nodo)
    friend std::ostream& operator<<(std::ostream& os, const BinaryTreeNodeBase& node) {
        os << node.m_data;
        return os;
    }
};

template <typename T>
class BinaryTreeNode : public BinaryTreeNodeBase<BinaryTreeNode<T>, T> {
public:
    BinaryTreeNode(T data) : BinaryTreeNodeBase<BinaryTreeNode<T>, T>(data) {}
};

// 4. forward iterator (inorder)
template <typename C>
class BTInorderForwardIterator : public general_iterator<C, BTInorderForwardIterator<C>> {
    using Node = typename C::Node;
    std::stack<Node*> m_stack;
    void push_left(Node* curr) {
        while (curr) {
            m_stack.push(curr);
            curr = curr->m_pChild[0];
        }
    }
public:
    BTInorderForwardIterator(C* container, Node* root)
        : general_iterator<C, BTInorderForwardIterator<C>>(container, nullptr) {
        push_left(root);
        if (!m_stack.empty()) this->m_pNode = m_stack.top();
    }
    BTInorderForwardIterator& operator++() {
        Node* curr = m_stack.top(); m_stack.pop();
        push_left(curr->m_pChild[1]);
        this->m_pNode = m_stack.empty() ? nullptr : m_stack.top();
        return *this;
    }
};

// 5. backward iterator (inorder)
template <typename C>
class BTInorderBackwardIterator : public general_iterator<C, BTInorderBackwardIterator<C>> {
    using Node = typename C::Node;
    std::stack<Node*> m_stack;
    void push_right(Node* curr) {
        while (curr) {
            m_stack.push(curr);
            curr = curr->m_pChild[1];
        }
    }
public:
    BTInorderBackwardIterator(C* container, Node* root)
        : general_iterator<C, BTInorderBackwardIterator<C>>(container, nullptr) {
        push_right(root);
        if (!m_stack.empty()) this->m_pNode = m_stack.top();
    }
    BTInorderBackwardIterator& operator++() {
        Node* curr = m_stack.top(); m_stack.pop();
        push_right(curr->m_pChild[0]);
        this->m_pNode = m_stack.empty() ? nullptr : m_stack.top();
        return *this;
    }
};

// 11. forward iterator (preorder)
template <typename C>
class BTPreorderForwardIterator : public general_iterator<C, BTPreorderForwardIterator<C>> {
    using Node = typename C::Node;
    std::stack<Node*> m_stack;
public:
    BTPreorderForwardIterator(C* container, Node* root)
        : general_iterator<C, BTPreorderForwardIterator<C>>(container, nullptr) {
        if (root) m_stack.push(root);
        if (!m_stack.empty()) this->m_pNode = m_stack.top();
    }
    BTPreorderForwardIterator& operator++() {
        Node* curr = m_stack.top(); m_stack.pop();
        if (curr->m_pChild[1]) m_stack.push(curr->m_pChild[1]);
        if (curr->m_pChild[0]) m_stack.push(curr->m_pChild[0]);
        this->m_pNode = m_stack.empty() ? nullptr : m_stack.top();
        return *this;
    }
};

// 12. backward iterator (preorder) - ahora con pila, sin vector
template <typename C>
class BTPreorderBackwardIterator : public general_iterator<C, BTPreorderBackwardIterator<C>> {
    using Node = typename C::Node;
    std::stack<Node*> m_stack;
public:
    BTPreorderBackwardIterator(C* container, Node* root)
        : general_iterator<C, BTPreorderBackwardIterator<C>>(container, nullptr) {
        if (!root) return;
        // Se recorre en preorder y se apilan los nodos en orden directo
        std::stack<Node*> traverse;
        traverse.push(root);
        while (!traverse.empty()) {
            Node* curr = traverse.top(); traverse.pop();
            m_stack.push(curr);
            // primero derecha, luego izquierda para mantener orden preorder al sacarlos
            if (curr->m_pChild[1]) traverse.push(curr->m_pChild[1]);
            if (curr->m_pChild[0]) traverse.push(curr->m_pChild[0]);
        }
        // Ahora la cima de m_stack es el último nodo del preorder
        this->m_pNode = m_stack.empty() ? nullptr : m_stack.top();
    }
    BTPreorderBackwardIterator& operator++() {
        m_stack.pop();
        this->m_pNode = m_stack.empty() ? nullptr : m_stack.top();
        return *this;
    }
};

// 13. forward iterator (postorder)
template <typename C>
class BTPostorderForwardIterator : public general_iterator<C, BTPostorderForwardIterator<C>> {
    using Node = typename C::Node;
    std::stack<Node*> m_stack;
    void build(Node* root) {
        if (!root) return;
        std::stack<Node*> s1;
        s1.push(root);
        while (!s1.empty()) {
            Node* curr = s1.top(); s1.pop();
            m_stack.push(curr);
            if (curr->m_pChild[0]) s1.push(curr->m_pChild[0]);
            if (curr->m_pChild[1]) s1.push(curr->m_pChild[1]);
        }
    }
public:
    BTPostorderForwardIterator(C* container, Node* root)
        : general_iterator<C, BTPostorderForwardIterator<C>>(container, nullptr) {
        build(root);
        if (!m_stack.empty()) this->m_pNode = m_stack.top();
    }
    BTPostorderForwardIterator& operator++() {
        m_stack.pop();
        this->m_pNode = m_stack.empty() ? nullptr : m_stack.top();
        return *this;
    }
};

// 14. backward iterator (postorder)
template <typename C>
class BTPostorderBackwardIterator : public general_iterator<C, BTPostorderBackwardIterator<C>> {
    using Node = typename C::Node;
    std::stack<Node*> m_stack;
public:
    BTPostorderBackwardIterator(C* container, Node* root)
        : general_iterator<C, BTPostorderBackwardIterator<C>>(container, nullptr) {
        if (root) m_stack.push(root);
        if (!m_stack.empty()) this->m_pNode = m_stack.top();
    }
    BTPostorderBackwardIterator& operator++() {
        Node* curr = m_stack.top(); m_stack.pop();
        if (curr->m_pChild[0]) m_stack.push(curr->m_pChild[0]);
        if (curr->m_pChild[1]) m_stack.push(curr->m_pChild[1]);
        this->m_pNode = m_stack.empty() ? nullptr : m_stack.top();
        return *this;
    }
};

// clase binarytree
template<typename Trait>
class BinaryTree {
public:
    using value_type = typename Trait::value_type;
    using Node       = typename Trait::Node;
    using Comp       = typename Trait::Comp;
    using height_t   = typename Node::height_t;   // hereda el tipo de altura

protected:
    Node* m_pRoot;
    size_t m_size;
    Comp m_comp;
    mutable std::shared_mutex m_mtx;

    void destroy(Node* node) {
        if (node) {
            destroy(node->m_pChild[0]);
            destroy(node->m_pChild[1]);
            delete node;
        }
    }

    Node* copyTree(Node* node) {
        if (!node) return nullptr;
        Node* newNode = new Node(node->m_data);
        newNode->m_pChild[0] = copyTree(node->m_pChild[0]);
        newNode->m_pChild[1] = copyTree(node->m_pChild[1]);
        return newNode;
    }

    // altura interna (no depende de m_height de nodos especializados)
    height_t internal_height(Node* node) const {
        if (!node) return 0;
        return 1 + std::max(internal_height(node->m_pChild[0]),
                            internal_height(node->m_pChild[1]));
    }

    // formato inorder (usa el operator<< del nodo)
    void internal_format_inorder(Node* node, std::ostream& os, bool& first) const {
        if (!node) return;
        internal_format_inorder(node->m_pChild[0], os, first);
        if (!first) os << ",";
        os << *node;              // responsabilidad del nodo
        first = false;
        internal_format_inorder(node->m_pChild[1], os, first);
    }

    // formato preorder (para persistencia)
    void internal_format_preorder(Node* node, std::ostream& os, bool& first) const {
        if (!node) return;
        if (!first) os << ",";
        os << *node;              // responsabilidad del nodo
        first = false;
        internal_format_preorder(node->m_pChild[0], os, first);
        internal_format_preorder(node->m_pChild[1], os, first);
    }

public:
    BinaryTree() : m_pRoot(nullptr), m_size(0) {}

    virtual ~BinaryTree() {
        std::unique_lock<std::shared_mutex> lock(m_mtx);
        destroy(m_pRoot);
    }

    // Constructor copia
    BinaryTree(const BinaryTree& other): m_pRoot(nullptr),
          m_size(other.m_size),
          m_comp(other.m_comp){

        std::shared_lock<std::shared_mutex> lock(other.m_mtx);
        m_pRoot = copyTree(other.m_pRoot);
    }

    BinaryTree& operator=(const BinaryTree& other) {
        if (this != &other) {
            std::unique_lock<std::shared_mutex> lock1(m_mtx, std::defer_lock);
            std::shared_lock<std::shared_mutex> lock2(other.m_mtx, std::defer_lock);
            std::lock(lock1, lock2);
            destroy(m_pRoot);
            m_pRoot = copyTree(other.m_pRoot);
            m_size = other.m_size;
            m_comp = other.m_comp;
        }
        return *this;
    }

    // Move constructor (con std::exchange)
    BinaryTree(BinaryTree&& other) noexcept
        : m_pRoot(std::exchange(other.m_pRoot, nullptr)),
          m_size(std::exchange(other.m_size, 0)),
          m_comp(std::move(other.m_comp))
    {}

    BinaryTree& operator=(BinaryTree&& other) noexcept {
        if (this != &other) {
            std::unique_lock<std::shared_mutex> lock1(m_mtx, std::defer_lock);
            std::unique_lock<std::shared_mutex> lock2(other.m_mtx, std::defer_lock);
            std::lock(lock1, lock2);
            destroy(m_pRoot);
            m_pRoot = std::exchange(other.m_pRoot, nullptr);
            m_size  = std::exchange(other.m_size, 0);
            m_comp = std::move(other.m_comp);
        }
        return *this;
    }

protected:
    // Core de inserción (usando m_comp directamente)
    virtual Node* insert_node(value_type data, bool& inserted) {
        Node** pNode = &m_pRoot;
        while (*pNode != nullptr) {
            if ((*pNode)->m_data == data) {
                inserted = false;
                return *pNode;
            }
            auto branch = m_comp((*pNode)->m_data, data);
            pNode = &((*pNode)->m_pChild[branch]);
        }
        *pNode = new Node(data);
        m_size++;
        inserted = true;
        return *pNode;
    }

public:
    virtual void insert(value_type data) {
        std::unique_lock<std::shared_mutex> lock(m_mtx);
        bool inserted = false;
        insert_node(data, inserted);
    }

    // remove base
    virtual void remove(value_type data) {
        std::unique_lock<std::shared_mutex> lock(m_mtx);
        Node** pNode = &m_pRoot;
        while (*pNode && (*pNode)->m_data != data) {
            auto branch = m_comp((*pNode)->m_data, data);
            pNode = &((*pNode)->m_pChild[branch]);
        }
        if (!*pNode) return;

        Node* target = *pNode;
        if (!target->m_pChild[0] && !target->m_pChild[1]) {
            *pNode = nullptr;
            delete target;
        } else if (!target->m_pChild[0]) {
            *pNode = target->m_pChild[1];
            delete target;
        } else if (!target->m_pChild[1]) {
            *pNode = target->m_pChild[0];
            delete target;
        } else {
            Node** pSucc = &(target->m_pChild[1]);
            while ((*pSucc)->m_pChild[0])
                pSucc = &((*pSucc)->m_pChild[0]);
            target->m_data = (*pSucc)->m_data;
            Node* succ = *pSucc;
            *pSucc = succ->m_pChild[1];
            delete succ;
        }
        m_size--;
    }

    // Búsqueda que retorna nodo (requisito del profesor)
    Node* find_node(value_type data) const {
        std::shared_lock<std::shared_mutex> lock(m_mtx);
        Node* curr = m_pRoot;
        while (curr) {
            if (curr->m_data == data) return curr;
            auto branch = m_comp(curr->m_data, data);
            curr = curr->m_pChild[branch];
        }
        return nullptr;
    }

    bool contains(value_type data) const {
        return find_node(data) != nullptr;
    }

    size_t size() const {
        std::shared_lock<std::shared_mutex> lock(m_mtx);
        return m_size;
    }

    height_t height() const {
        std::shared_lock<std::shared_mutex> lock(m_mtx);
        return internal_height(m_pRoot);
    }

    std::string ToString() const {
        std::shared_lock<std::shared_mutex> lock(m_mtx);
        std::ostringstream ss;
        ss << "[";
        bool first = true;
        internal_format_inorder(m_pRoot, ss, first);
        ss << "]";
        return ss.str();
    }

    // Persistencia en PREORDER (preserva estructura)
    friend std::ostream& operator<<(std::ostream& os, const BinaryTree& tree) {
        std::shared_lock<std::shared_mutex> lock(tree.m_mtx);
        os << "[";
        bool first = true;
        tree.internal_format_preorder(tree.m_pRoot, os, first);
        os << "]";
        return os;
    }

    friend std::istream& operator>>(std::istream& is, BinaryTree& tree) {
        std::unique_lock<std::shared_mutex> lock(tree.m_mtx);

        tree.destroy(tree.m_pRoot);
        tree.m_pRoot = nullptr;
        tree.m_size = 0;

        char c;

        if (is >> c && c == '[') {
            value_type val;

            while (is >> val) {

                bool inserted = false;
                tree.insert_node(val, inserted);

                is >> c;

                if (c == ']')
                    break;
            }
        }

        return is;
    }

    // ForEach sin const_cast
    template <typename Func, typename... Args>
    void ForEach(Func func, Args&&... args) {
        std::shared_lock<std::shared_mutex> lock(m_mtx);
        for (auto it = begin(); it != end(); ++it) {
            func(*it, std::forward<Args>(args)...);
        }
    }

    // Tipos de iteradores
    using forward_iterator = BTInorderForwardIterator<BinaryTree<Trait>>;
    using backward_iterator = BTInorderBackwardIterator<BinaryTree<Trait>>;
    using preorder_forward_iterator = BTPreorderForwardIterator<BinaryTree<Trait>>;
    using preorder_backward_iterator = BTPreorderBackwardIterator<BinaryTree<Trait>>;
    using postorder_forward_iterator = BTPostorderForwardIterator<BinaryTree<Trait>>;
    using postorder_backward_iterator = BTPostorderBackwardIterator<BinaryTree<Trait>>;

    forward_iterator begin() { return forward_iterator(this, m_pRoot); }
    forward_iterator end()   { return forward_iterator(this, nullptr); }

    backward_iterator rbegin() { return backward_iterator(this, m_pRoot); }
    backward_iterator rend()   { return backward_iterator(this, nullptr); }

    preorder_forward_iterator preorder_begin() { return preorder_forward_iterator(this, m_pRoot); }
    preorder_forward_iterator preorder_end()   { return preorder_forward_iterator(this, nullptr); }

    preorder_backward_iterator preorder_rbegin() { return preorder_backward_iterator(this, m_pRoot); }
    preorder_backward_iterator preorder_rend()   { return preorder_backward_iterator(this, nullptr); }

    postorder_forward_iterator postorder_begin() { return postorder_forward_iterator(this, m_pRoot); }
    postorder_forward_iterator postorder_end()   { return postorder_forward_iterator(this, nullptr); }

    postorder_backward_iterator postorder_rbegin() { return postorder_backward_iterator(this, m_pRoot); }
    postorder_backward_iterator postorder_rend()   { return postorder_backward_iterator(this, nullptr); }

    // find retorna iterador
    forward_iterator find(value_type data) {
        Node* node = find_node(data);
        if (node) return forward_iterator(this, node);
        return end();
    }
};

#endif // __BINARYTREE_H__