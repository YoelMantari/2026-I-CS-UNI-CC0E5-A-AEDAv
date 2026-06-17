#ifndef __BINARYTREEAVL_H__
#define __BINARYTREEAVL_H__

#include "BinaryTree.h"
#include "types.h"
#include <algorithm>
#include <cstddef>

// Nodo AVL usando CRTP
template <typename T>
class AVLNode : public BinaryTreeNodeBase<AVLNode<T>, T> {
public:
    Ref m_ref;
    std::size_t m_height;

    AVLNode() : BinaryTreeNodeBase<AVLNode<T>, T>(T{}), m_ref(Ref{}), m_height(1) {}
    explicit AVLNode(T data, Ref ref = Ref{})
        : BinaryTreeNodeBase<AVLNode<T>, T>(data), m_ref(ref), m_height(1) {}

    Ref& getRef() { return m_ref; }
    const Ref& getRef() const { return m_ref; }
    void setRef(Ref ref) { m_ref = ref; }
};

template <typename Trait>
class BinaryTreeAVL : public BinaryTree<Trait> {
    using value_type = typename Trait::value_type;
    using Node       = typename Trait::Node;
    using Comp       = typename Trait::Comp;
    using height_t   = typename BinaryTree<Trait>::height_t;

private:
    std::size_t node_height(const Node* node) const {
        return node ? node->m_height : 0;
    }

    void update_height(Node* node) {
        if (node) {
            node->m_height = 1 + std::max(node_height(node->m_pChild[0]),
                                           node_height(node->m_pChild[1]));
        }
    }

    Node* min_value_node(Node* node) {
        while (node && node->m_pChild[0])
            node = node->m_pChild[0];
        return node;
    }

    int balance_factor(const Node* node) const {
        if (!node) return 0;
        return static_cast<int>(node_height(node->m_pChild[0])
                              - node_height(node->m_pChild[1]));
    }

    // dir == true  → rota a la izquierda (sube hijo derecho)
    // dir == false → rota a la derecha   (sube hijo izquierdo)
    Node* rotate(Node* oldRoot, bool dir) {
        auto other    = dir ? 1 : 0;
        auto opposite = 1 - other;
        Node* newRoot = oldRoot->m_pChild[other];
        oldRoot->m_pChild[other] = newRoot->m_pChild[opposite];
        newRoot->m_pChild[opposite] = oldRoot;
        update_height(oldRoot);
        update_height(newRoot);
        return newRoot;
    }

    Node* rebalance(Node* node) {
        update_height(node);

        int balance = balance_factor(node);

        if (balance > 1) {
            if (balance_factor(node->m_pChild[0]) < 0)
                node->m_pChild[0] = rotate(node->m_pChild[0], true);
            return rotate(node, false);
        }

        if (balance < -1) {
            if (balance_factor(node->m_pChild[1]) > 0)
                node->m_pChild[1] = rotate(node->m_pChild[1], false);
            return rotate(node, true);
        }

        return node;
    }

    Node* insert_recursive(Node* node, value_type data, bool& inserted) {
        if (!node) {
            inserted = true;
            return new Node(data);
        }

        if (node->m_data == data) {
            inserted = false;
            return node;
        }

        auto branch = this->m_comp(node->m_data, data);
        node->m_pChild[branch] = insert_recursive(node->m_pChild[branch], data, inserted);
        return inserted ? rebalance(node) : node;
    }

    Node* remove_recursive(Node* node, value_type data, bool& removed) {
        if (!node)
            return nullptr;

        if (data != node->m_data) {
            auto branch = this->m_comp(node->m_data, data);
            node->m_pChild[branch] =
                remove_recursive(node->m_pChild[branch], data, removed);
        } else {
            removed = true;

            if (!node->m_pChild[0] || !node->m_pChild[1]) {
                Node* child = node->m_pChild[0] ? node->m_pChild[0] : node->m_pChild[1];
                delete node;
                return child;
            }

            Node* succ = min_value_node(node->m_pChild[1]);
            node->m_data = succ->m_data;
            node->setRef(succ->getRef());

            bool dummy = false;
            node->m_pChild[1] =
                remove_recursive(node->m_pChild[1], succ->m_data, dummy);
        }

        return rebalance(node);
    }

protected:
    Node* copyTree(Node* node) override {
        if (!node) return nullptr;
        Node* newNode = new Node(node->m_data, node->getRef());
        newNode->m_pChild[0] = copyTree(node->m_pChild[0]);
        newNode->m_pChild[1] = copyTree(node->m_pChild[1]);
        update_height(newNode);
        return newNode;
    }

    Node* insert_node(value_type data, bool& inserted) override {
        inserted = false;
        this->m_pRoot = insert_recursive(this->m_pRoot, data, inserted);
        if (inserted) this->m_size++;
        return this->find_node_impl(data);
    }

public:
    BinaryTreeAVL() : BinaryTree<Trait>() {}

    BinaryTreeAVL(const BinaryTreeAVL& other) : BinaryTree<Trait>() {
        std::shared_lock<std::shared_mutex> lock(other.m_mtx);
        this->m_size  = other.m_size;
        this->m_comp  = other.m_comp;
        this->m_pRoot = copyTree(other.m_pRoot);
    }

    BinaryTreeAVL& operator=(const BinaryTreeAVL& other) {
        if (this != &other) {
            std::unique_lock<std::shared_mutex> lock1(this->m_mtx, std::defer_lock);
            std::shared_lock<std::shared_mutex> lock2(other.m_mtx, std::defer_lock);
            std::lock(lock1, lock2);
            this->destroy(this->m_pRoot);
            this->m_pRoot = copyTree(other.m_pRoot);
            this->m_size  = other.m_size;
            this->m_comp  = other.m_comp;
        }
        return *this;
    }

    void insert(value_type data) override {
        std::unique_lock<std::shared_mutex> lock(this->m_mtx);
        bool inserted = false;
        insert_node(data, inserted);
    }

    void remove(value_type data) override {
        std::unique_lock<std::shared_mutex> lock(this->m_mtx);

        bool removed = false;
        this->m_pRoot = remove_recursive(this->m_pRoot, data, removed);
        if (removed) this->m_size--;
    }

    height_t height() const override {
        std::shared_lock<std::shared_mutex> lock(this->m_mtx);
        return this->m_pRoot ? node_height(this->m_pRoot) : 0;
    }
};

#endif // __BINARYTREEAVL_H__
