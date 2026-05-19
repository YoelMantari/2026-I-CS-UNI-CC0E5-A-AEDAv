#ifndef __BINARYTREEAVL_H__
#define __BINARYTREEAVL_H__

#include "BinaryTree.h"
#include <algorithm>
#include <cstddef>

// Nodo AVL usando CRTP
template <typename T>
class AVLNode : public BinaryTreeNodeBase<AVLNode<T>, T> {
public:
    std::size_t m_height;   // altura del nodo (tipo simple, sin propagar)
    AVLNode(T data) : BinaryTreeNodeBase<AVLNode<T>, T>(data), m_height(1) {}
};

// Árbol AVL heredado de BinaryTree
template <typename Trait>
class BinaryTreeAVL : public BinaryTree<Trait> {
    using value_type = typename Trait::value_type;
    using Node       = typename Trait::Node;
    using Comp       = typename Trait::Comp;

private:
    // altura de un nodo (usa el campo m_height del nodo AVL)
    auto node_height(const Node* node) const {
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

    // factor de balance (usa auto para no propagar tipo)
    auto balance_factor(const Node* node) const {
        if (!node) return 0;
        return node_height(node->m_pChild[0]) - node_height(node->m_pChild[1]);
    }

    // Rotación unificada (elimina código duplicado)
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

        auto balance = balance_factor(node);

        // izquierda pesada
        if (balance > 1) {

            auto leftBalance =
                balance_factor(node->m_pChild[0]);

            // izquierda-derecha
            if (leftBalance < 0)
                node->m_pChild[0] =
                    rotate(node->m_pChild[0], true);

            // izquierda-izquierda
            return rotate(node, false);
        }

        // derecha pesada
        if (balance < -1) {

            auto rightBalance =
                balance_factor(node->m_pChild[1]);

            // derecha-izquierda
            if (rightBalance > 0)
                node->m_pChild[1] =
                    rotate(node->m_pChild[1], false);

            // derecha-derecha
            return rotate(node, true);
        }

        return node;
    }

    // Inserción recursiva balanceada (eficiente, no recorre desde la raíz)
    Node* insert_recursive(Node* node, value_type data, bool& inserted) {
        if (!node) {
            node = new Node(data);
            inserted = true;
            return node;
        }

        auto branch = this->m_comp(node->m_data, data);
        node->m_pChild[branch] = insert_recursive(node->m_pChild[branch], data, inserted);

        update_height(node);
        auto balance = balance_factor(node);

        // Rebalanceo
        if (balance > 1) {                         // lado izquierdo pesado
            auto childBranch = this->m_comp(node->m_pChild[0]->m_data, data);
            if (childBranch == 0)                  // izquierda-izquierda
                return rotate(node, false);        // rota derecha
            else {                                 // izquierda-derecha
                node->m_pChild[0] = rotate(node->m_pChild[0], true);
                return rotate(node, false);
            }
        }
        if (balance < -1) {                        // lado derecho pesado
            auto childBranch = this->m_comp(node->m_pChild[1]->m_data, data);
            if (childBranch == 1)                  // derecha-derecha
                return rotate(node, true);         // rota izquierda
            else {                                 // derecha-izquierda
                node->m_pChild[1] = rotate(node->m_pChild[1], false);
                return rotate(node, true);
            }
        }
        return node;
    }

    Node* remove_recursive(
            Node* node,
            value_type data,
            bool& removed) {

        if (!node)
            return nullptr;

        if (data != node->m_data) {

            auto branch =
                this->m_comp(node->m_data, data);

            node->m_pChild[branch] =
                remove_recursive(
                    node->m_pChild[branch],
                    data,
                    removed);
        }
        else {

            removed = true;

            // nodo con 0 o 1 hijo
            if (!node->m_pChild[0] ||
                !node->m_pChild[1]) {

                Node* child =
                    node->m_pChild[0]
                        ? node->m_pChild[0]
                        : node->m_pChild[1];

                delete node;
                return child;
            }

            // nodo con 2 hijos
            Node* succ =
                min_value_node(node->m_pChild[1]);

            node->m_data = succ->m_data;

            bool dummy = false;

            node->m_pChild[1] =
                remove_recursive(
                    node->m_pChild[1],
                    succ->m_data,
                    dummy);
        }

        return rebalance(node);
    }

public:
    BinaryTreeAVL() : BinaryTree<Trait>() {}

    void insert(value_type data) override {
        std::unique_lock<std::shared_mutex> lock(this->m_mtx);
        bool inserted = false;
        this->m_pRoot = insert_recursive(this->m_pRoot, data, inserted);
        if (inserted) this->m_size++;
    }

    void remove(value_type data) override {
        std::unique_lock<std::shared_mutex>
            lock(this->m_mtx);

        bool removed = false;

        this->m_pRoot =
            remove_recursive(
                this->m_pRoot,
                data,
                removed);

        if (removed)
            --this->m_size;
    }
};

#endif // __BINARYTREEAVL_H__