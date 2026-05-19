#ifndef __BINARYTREERB_H__
#define __BINARYTREERB_H__

#include "BinaryTree.h"

enum RBColor { RED, BLACK };

//nodo rbt uso crtp
template <typename T>
class RBTreeNode : public BinaryTreeNodeBase<RBTreeNode<T>, T> {
public:
    RBColor m_color;
    // arbol rojo-negro puntero al padre para rotar
    RBTreeNode* m_pParent;
    RBTreeNode(T data) : BinaryTreeNodeBase<RBTreeNode<T>, T>(data), m_color(RED), m_pParent(nullptr) {}
};

// arbol rbt, hereda de binarytree
template <typename Trait>
class BinaryTreeRB : public BinaryTree<Trait> {
    using value_type = typename Trait::value_type;
    using Node       = typename Trait::Node;
    using Comp       = typename Trait::Comp;

private:
    void rotate_left(Node* x) {
        Node* y = x->m_pChild[1];
        x->m_pChild[1] = y->m_pChild[0];
        if (y->m_pChild[0] != nullptr)
            y->m_pChild[0]->m_pParent = x;
        y->m_pParent = x->m_pParent;
        if (x->m_pParent == nullptr)
            this->m_pRoot = y;
        else if (x == x->m_pParent->m_pChild[0])
            x->m_pParent->m_pChild[0] = y;
        else
            x->m_pParent->m_pChild[1] = y;
        y->m_pChild[0] = x;
        x->m_pParent = y;
    }

    void rotate_right(Node* x) {
        Node* y = x->m_pChild[0];
        x->m_pChild[0] = y->m_pChild[1];
        if (y->m_pChild[1] != nullptr)
            y->m_pChild[1]->m_pParent = x;
        y->m_pParent = x->m_pParent;
        if (x->m_pParent == nullptr)
            this->m_pRoot = y;
        else if (x == x->m_pParent->m_pChild[1])
            x->m_pParent->m_pChild[1] = y;
        else
            x->m_pParent->m_pChild[0] = y;
        y->m_pChild[1] = x;
        x->m_pParent = y;
    }

    void fix_insert(Node* k) {
        Node* u;
        while (k->m_pParent != nullptr && k->m_pParent->m_color == RED) {
            if (k->m_pParent == k->m_pParent->m_pParent->m_pChild[1]) {
                u = k->m_pParent->m_pParent->m_pChild[0];
                if (u != nullptr && u->m_color == RED) {
                    u->m_color = BLACK;
                    k->m_pParent->m_color = BLACK;
                    k->m_pParent->m_pParent->m_color = RED;
                    k = k->m_pParent->m_pParent;
                } else {
                    if (k == k->m_pParent->m_pChild[0]) {
                        k = k->m_pParent;
                        rotate_right(k);
                    }
                    k->m_pParent->m_color = BLACK;
                    k->m_pParent->m_pParent->m_color = RED;
                    rotate_left(k->m_pParent->m_pParent);
                }
            } else {
                u = k->m_pParent->m_pParent->m_pChild[1];
                if (u != nullptr && u->m_color == RED) {
                    u->m_color = BLACK;
                    k->m_pParent->m_color = BLACK;
                    k->m_pParent->m_pParent->m_color = RED;
                    k = k->m_pParent->m_pParent;
                } else {
                    if (k == k->m_pParent->m_pChild[1]) {
                        k = k->m_pParent;
                        rotate_left(k);
                    }
                    k->m_pParent->m_color = BLACK;
                    k->m_pParent->m_pParent->m_color = RED;
                    rotate_right(k->m_pParent->m_pParent);
                }
            }
            if (k == this->m_pRoot) break;
        }
        this->m_pRoot->m_color = BLACK;
    }

public:
    BinaryTreeRB() : BinaryTree<Trait>() {}

    void insert(value_type data) override {
        std::unique_lock<std::shared_mutex> lock(this->m_mtx);
        bool inserted = false;
        Node* node = this->insert_node(data, inserted);
        if (!inserted) return; // evitar duplicados
        
        // El node ahora necesita estar enlazado a su padre correctamente y ajustado
        Node* curr = this->m_pRoot;
        Node* parent = nullptr;
        while (curr != node) {
            parent = curr;
            bool branch = this->m_comp(curr->m_data, data);
            curr = curr->m_pChild[branch];
        }
        node->m_pParent = parent;

        if (node->m_pParent == nullptr) {
            node->m_color = BLACK;
            return;
        }

        if (node->m_pParent->m_pParent == nullptr) {
            return;
        }

        fix_insert(node);
    }
};

#endif // __BINARYTREERB_H__
