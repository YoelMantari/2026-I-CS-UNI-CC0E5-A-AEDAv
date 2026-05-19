#ifndef __BINARYTREEAVLDEMO_H__
#define __BINARYTREEAVLDEMO_H__

#include <iostream>
#include <fstream>
#include <typeinfo>
#include "../BinaryTreeAVL.h"
#include "../containers/traits.h"

using namespace std;

template <typename T>
void DemoAVL() {
    cout << "=== Probar AVL Tree con tipo: " << typeid(T).name() << " ===" << endl;
    BinaryTreeAVL<AscendingAVLTrait<T>> miAVL;
    for (int i = 1; i <= 7; ++i) {
        miAVL.insert(T(i));
    }
    
    cout << "AVL Inorder Fwd (Range-based for): ";
    for (auto val : miAVL) cout << val << " ";
    cout << endl;

    cout << "AVL Inorder Bwd: ";
    for (auto it = miAVL.rbegin(); it != miAVL.rend(); ++it) cout << *it << " ";
    cout << endl;

    cout << "AVL Preorder Fwd: ";
    for (auto it = miAVL.preorder_begin(); it != miAVL.preorder_end(); ++it) cout << *it << " ";
    cout << endl;

    cout << "AVL Preorder Bwd: ";
    for (auto it = miAVL.preorder_rbegin(); it != miAVL.preorder_rend(); ++it) cout << *it << " ";
    cout << endl;

    cout << "AVL Postorder Fwd: ";
    for (auto it = miAVL.postorder_begin(); it != miAVL.postorder_end(); ++it) cout << *it << " ";
    cout << endl;

    cout << "AVL Postorder Bwd: ";
    for (auto it = miAVL.postorder_rbegin(); it != miAVL.postorder_rend(); ++it) cout << *it << " ";
    cout << endl;

    // Test ToString
    cout << "AVL ToString: " << miAVL.ToString() << endl;

    // Test Persistencia
    ofstream out("avltree.txt");
    out << miAVL;
    out.close();

    BinaryTreeAVL<AscendingAVLTrait<T>> miAVL2;
    ifstream in("avltree.txt");
    in >> miAVL2;
    in.close();
    cout << "AVL Leido desde archivo: " << miAVL2.ToString() << endl;

    // Test altura (característica propia de AVL)
    cout << "AVL Altura: " << miAVL2.height() << endl;

    // Constructor copia
    BinaryTreeAVL<AscendingAVLTrait<T>> miAVL3 = miAVL2;
    cout << "AVL Copia: " << miAVL3.ToString() << endl;

    // Move constructor
    BinaryTreeAVL<AscendingAVLTrait<T>> miAVL4 = std::move(miAVL2);
    cout << "AVL Move: " << miAVL4.ToString() << endl;
    cout << "AVL Original post-move (vacio): " << miAVL2.ToString() << endl;

    // Test contains
    cout << "AVL Contains(T(4)): " << (miAVL.contains(T(4)) ? "true" : "false") << endl;
    cout << "AVL Contains(T(10)): " << (miAVL.contains(T(10)) ? "true" : "false") << endl;

    // Test remove (balanceado)
    cout << "AVL Remove(T(4))..." << endl;
    miAVL.remove(T(4));
    cout << "AVL Post-remove: " << miAVL.ToString() << endl;
    cout << "AVL Altura post-remove: " << miAVL.height() << "\n\n";
}

#endif // __BINARYTREEAVLDEMO_H__