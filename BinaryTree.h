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
#include <memory>
#include <mutex>
#include <shared_mutex>
#include "containers/general_iterator.h"

template<typename C>
using BTReadLock = std::shared_ptr<std::shared_lock<std::shared_mutex>>;

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

enum class BTTraversal { Inorder, Preorder, Postorder };
enum class BTDirection { Forward, Backward };

// Iterador unificado: recorrido x dirección (sustituye las 6 clases anteriores)
template <typename C>
class BTIterator : public general_iterator<C, BTIterator<C>> {
    using Node = typename C::Node;
    std::stack<Node*> m_stack;
    BTReadLock<C> m_readLock;
    BTTraversal m_traversal;
    BTDirection m_direction;

    void push_spine(Node* curr, int side) {
        while (curr) {
            m_stack.push(curr);
            curr = curr->m_pChild[side];
        }
    }

    void build_preorder_backward(Node* root) {
        if (!root) return;
        std::stack<Node*> traverse;
        traverse.push(root);
        while (!traverse.empty()) {
            Node* curr = traverse.top(); traverse.pop();
            m_stack.push(curr);
            if (curr->m_pChild[1]) traverse.push(curr->m_pChild[1]);
            if (curr->m_pChild[0]) traverse.push(curr->m_pChild[0]);
        }
    }

    void build_postorder_forward(Node* root) {
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

    void update_node() {
        this->m_pNode = m_stack.empty() ? nullptr : m_stack.top();
    }

    bool is_prebuilt() const {
        return (m_traversal == BTTraversal::Preorder && m_direction == BTDirection::Backward)
            || (m_traversal == BTTraversal::Postorder && m_direction == BTDirection::Forward);
    }

    void init(Node* root) {
        if (!root) {
            update_node();
            return;
        }
        switch (m_traversal) {
        case BTTraversal::Inorder:
            push_spine(root, m_direction == BTDirection::Forward ? 0 : 1);
            break;
        case BTTraversal::Preorder:
            if (m_direction == BTDirection::Forward)
                m_stack.push(root);
            else
                build_preorder_backward(root);
            break;
        case BTTraversal::Postorder:
            if (m_direction == BTDirection::Forward)
                build_postorder_forward(root);
            else
                m_stack.push(root);
            break;
        }
        update_node();
    }

public:
    BTIterator(C* container, Node* root, BTTraversal traversal, BTDirection direction,
               BTReadLock<C> lock = nullptr)
        : general_iterator<C, BTIterator<C>>(container, nullptr),
          m_readLock(std::move(lock)),
          m_traversal(traversal),
          m_direction(direction) {
        init(root);
    }

    BTIterator& operator++() {
        if (m_stack.empty()) return *this;

        Node* curr = m_stack.top();
        m_stack.pop();

        if (!is_prebuilt()) {
            switch (m_traversal) {
            case BTTraversal::Inorder:
                push_spine(curr->m_pChild[m_direction == BTDirection::Forward ? 1 : 0],
                           m_direction == BTDirection::Forward ? 0 : 1);
                break;
            case BTTraversal::Preorder:
                if (curr->m_pChild[1]) m_stack.push(curr->m_pChild[1]);
                if (curr->m_pChild[0]) m_stack.push(curr->m_pChild[0]);
                break;
            case BTTraversal::Postorder:
                if (curr->m_pChild[0]) m_stack.push(curr->m_pChild[0]);
                if (curr->m_pChild[1]) m_stack.push(curr->m_pChild[1]);
                break;
            }
        }
        update_node();
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
    using height_t   = typename Node::height_t;
    using ReadLock   = BTReadLock<BinaryTree<Trait>>;
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

    virtual Node* copyTree(Node* node) {
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

    Node* find_node_impl(value_type data) const {
        Node* curr = m_pRoot;
        while (curr) {
            if (curr->m_data == data) return curr;
            auto branch = m_comp(curr->m_data, data);
            curr = curr->m_pChild[branch];
        }
        return nullptr;
    }

    ReadLock make_read_lock() const {
        return std::make_shared<std::shared_lock<std::shared_mutex>>(m_mtx);
    }

    using tree_iterator = BTIterator<BinaryTree<Trait>>;

    tree_iterator make_iter(Node* root, BTTraversal traversal, BTDirection direction,
                            ReadLock lock = nullptr) {
        return tree_iterator(this, root, traversal, direction, std::move(lock));
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
        : m_pRoot(nullptr), m_size(0), m_comp() {
        std::unique_lock<std::shared_mutex> lock(other.m_mtx);
        m_pRoot = std::exchange(other.m_pRoot, nullptr);
        m_size  = std::exchange(other.m_size, 0);
        m_comp  = std::move(other.m_comp);
    }

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
        return find_node_impl(data);
    }

    bool contains(value_type data) const {
        return find_node(data) != nullptr;
    }

    size_t size() const {
        std::shared_lock<std::shared_mutex> lock(m_mtx);
        return m_size;
    }

    virtual height_t height() const {
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

    // ForEach sin const_cast (el lock lo mantiene begin() en el iterador)
    template <typename Func, typename... Args>
    void ForEach(Func func, Args&&... args) {
        for (auto it = begin(); it != end(); ++it) {
            func(*it, std::forward<Args>(args)...);
        }
    }

    // Alias semánticos (mismo tipo subyacente)
    using forward_iterator              = tree_iterator;
    using backward_iterator             = tree_iterator;
    using preorder_forward_iterator     = tree_iterator;
    using preorder_backward_iterator    = tree_iterator;
    using postorder_forward_iterator    = tree_iterator;
    using postorder_backward_iterator   = tree_iterator;

    forward_iterator begin() {
        return make_iter(m_pRoot, BTTraversal::Inorder, BTDirection::Forward, make_read_lock());
    }
    forward_iterator end() {
        return make_iter(nullptr, BTTraversal::Inorder, BTDirection::Forward);
    }

    backward_iterator rbegin() {
        return make_iter(m_pRoot, BTTraversal::Inorder, BTDirection::Backward, make_read_lock());
    }
    backward_iterator rend() {
        return make_iter(nullptr, BTTraversal::Inorder, BTDirection::Backward);
    }

    preorder_forward_iterator preorder_begin() {
        return make_iter(m_pRoot, BTTraversal::Preorder, BTDirection::Forward, make_read_lock());
    }
    preorder_forward_iterator preorder_end() {
        return make_iter(nullptr, BTTraversal::Preorder, BTDirection::Forward);
    }

    preorder_backward_iterator preorder_rbegin() {
        return make_iter(m_pRoot, BTTraversal::Preorder, BTDirection::Backward, make_read_lock());
    }
    preorder_backward_iterator preorder_rend() {
        return make_iter(nullptr, BTTraversal::Preorder, BTDirection::Backward);
    }

    postorder_forward_iterator postorder_begin() {
        return make_iter(m_pRoot, BTTraversal::Postorder, BTDirection::Forward, make_read_lock());
    }
    postorder_forward_iterator postorder_end() {
        return make_iter(nullptr, BTTraversal::Postorder, BTDirection::Forward);
    }

    postorder_backward_iterator postorder_rbegin() {
        return make_iter(m_pRoot, BTTraversal::Postorder, BTDirection::Backward, make_read_lock());
    }
    postorder_backward_iterator postorder_rend() {
        return make_iter(nullptr, BTTraversal::Postorder, BTDirection::Backward);
    }

    std::pair<tree_iterator, tree_iterator> range(BTTraversal traversal, BTDirection direction) {
        return {make_iter(m_pRoot, traversal, direction, make_read_lock()),
                make_iter(nullptr, traversal, direction)};
    }

    forward_iterator find(value_type data) {
        auto lock = make_read_lock();
        Node* node = find_node_impl(data);
        if (node) return make_iter(node, BTTraversal::Inorder, BTDirection::Forward, std::move(lock));
        return end();
    }
};

#endif // __BINARYTREE_H__