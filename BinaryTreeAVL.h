#ifndef __BINARYTREEAVL_H__
#define __BINARYTREEAVL_H__

#include "BinaryTree.h"
#include <algorithm>

//nodo avl uso de crtp
template <typename T>
class AVLNode : public BinaryTreeNodeBase<AVLNode<T>, T> {
public:
    int m_height;
    AVLNode(T data) : BinaryTreeNodeBase<AVLNode<T>, T>(data), m_height(1) {}
};

// arbol avl heredada de binarytree
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

public:
    BinaryTreeAVL() : BinaryTree<Trait>() {}

    // Sobreescritura polimórfica (override) de la inserción, reutilizando el base insert
    void insert(value_type data) override {
        std::unique_lock<std::shared_mutex> lock(this->m_mtx);
        
        bool inserted = false;
        Node* node = this->insert_node(data, inserted);
        if (!inserted) return; 

        // Ahora tenemos que re-balancear trazando la ruta desde la raíz
        std::vector<Node*> path;
        Node* curr = this->m_pRoot;
        while (curr != node) {
            path.push_back(curr);
            bool branch = !this->m_comp(curr->m_data, data);
            curr = curr->m_pChild[branch];
        }
        path.push_back(node);

        // Actualizar alturas y balancear de abajo hacia arriba iterativamente
        for (int i = (int)path.size() - 1; i >= 0; i--) {
            Node* current = path[i];
            update_height(current);
            int bf = balance_factor(current);

            Node* new_subtree = current;

            // Desbalance en rama izquierda (pChild[0])
            if (bf > 1) {
                bool child_branch = !this->m_comp(current->m_pChild[0]->m_data, data);
                if (child_branch == 0) {
                    new_subtree = rotate_right(current);
                } else {
                    current->m_pChild[0] = rotate_left(current->m_pChild[0]);
                    new_subtree = rotate_right(current);
                }
            }
            // Desbalance en rama derecha (pChild[1])
            else if (bf < -1) {
                bool child_branch = !this->m_comp(current->m_pChild[1]->m_data, data);
                if (child_branch == 1) {
                    new_subtree = rotate_left(current);
                } else {
                    current->m_pChild[1] = rotate_right(current->m_pChild[1]);
                    new_subtree = rotate_left(current);
                }
            }

            // reconectar si hubo rotacion
            if (new_subtree != current) {
                if (i == 0) {
                    this->m_pRoot = new_subtree;
                } else {
                    Node* parent = path[i-1];
                    bool parent_branch = (parent->m_pChild[1] == current);
                    parent->m_pChild[parent_branch] = new_subtree;
                }
            }
        }
    }
};

#endif // __BINARYTREEAVL_H__
