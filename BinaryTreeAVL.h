#ifndef __BINARYTREEAVL_H__
#define __BINARYTREEAVL_H__

#include "BinaryTree.h"
#include <algorithm>

// ========================================================
// 1. Nodo AVL (usa CRTP)
// ========================================================
template <typename T>
class AVLNode : public BinaryTreeNodeBase<AVLNode<T>, T> {
public:
    int m_height;
    AVLNode(T data) : BinaryTreeNodeBase<AVLNode<T>, T>(data), m_height(1) {}
};

// ========================================================
// 2. Árbol AVL (Hereda de BinaryTree)
// ========================================================
template <typename Trait>
class BinaryTreeAVL : public BinaryTree<Trait> {
    using value_type = typename Trait::value_type;
    using Node       = typename Trait::Node;
    using Comp       = typename Trait::Comp;

private:
    int get_height(Node* node) const {
        return node ? node->m_height : 0;
    }

    void update_height(Node* node) {
        if (node) {
            node->m_height = 1 + std::max(get_height(node->m_pChild[0]), get_height(node->m_pChild[1]));
        }
    }

    int balance_factor(Node* node) const {
        if (!node) return 0;
        return get_height(node->m_pChild[0]) - get_height(node->m_pChild[1]);
    }

    Node* rotate_left(Node* x) {
        Node* y = x->m_pChild[1];
        Node* T2 = y->m_pChild[0];
        
        y->m_pChild[0] = x;
        x->m_pChild[1] = T2;
        
        update_height(x);
        update_height(y);
        return y;
    }

    Node* rotate_right(Node* y) {
        Node* x = y->m_pChild[0];
        Node* T2 = x->m_pChild[1];
        
        x->m_pChild[1] = y;
        y->m_pChild[0] = T2;
        
        update_height(y);
        update_height(x);
        return x;
    }

    Node* avl_insert(Node* node, value_type data, bool& inserted) {
        if (!node) {
            inserted = true;
            return new Node(data);
        }

        if (node->m_data == data) {
            inserted = false;
            return node;
        }

        // Determinar rama usando la misma convención del BinaryTree
        bool branch = !this->m_comp(node->m_data, data);
        node->m_pChild[branch] = avl_insert(node->m_pChild[branch], data, inserted);

        update_height(node);
        int bf = balance_factor(node);

        // Desbalance en rama izquierda (pChild[0])
        if (bf > 1) {
            bool child_branch = !this->m_comp(node->m_pChild[0]->m_data, data);
            if (child_branch == 0) {
                // Caso LL (Left-Left)
                return rotate_right(node);
            } else {
                // Caso LR (Left-Right)
                node->m_pChild[0] = rotate_left(node->m_pChild[0]);
                return rotate_right(node);
            }
        }
        
        // Desbalance en rama derecha (pChild[1])
        if (bf < -1) {
            bool child_branch = !this->m_comp(node->m_pChild[1]->m_data, data);
            if (child_branch == 1) {
                // Caso RR (Right-Right)
                return rotate_left(node);
            } else {
                // Caso RL (Right-Left)
                node->m_pChild[1] = rotate_right(node->m_pChild[1]);
                return rotate_left(node);
            }
        }

        return node;
    }

public:
    // Configura el constructor llamando al base
    BinaryTreeAVL() : BinaryTree<Trait>() {}

    // Sobreescritura polimórfica (override) de la inserción
    void insert(value_type data) override {
        std::unique_lock<std::shared_mutex> lock(this->m_mtx);
        bool inserted = false;
        this->m_pRoot = avl_insert(this->m_pRoot, data, inserted);
        if (inserted) {
            this->m_size++;
        }
    }
};

#endif // __BINARYTREEAVL_H__
