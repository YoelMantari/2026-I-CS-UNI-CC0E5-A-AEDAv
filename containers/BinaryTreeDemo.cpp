#ifndef __BINARYTREEDEMO_H__
#define __BINARYTREEDEMO_H__

#include <iostream>
#include <fstream>
#include <typeinfo>
#include "../BinaryTree.h"
#include "../containers/traits.h"

using namespace std;

// Función PrintElement generalizada
template <typename T, typename U>
void PrintElement(T& el, U multiplier) {
    cout << (el * multiplier) << " ";
}

// Especialización para string (no se multiplica, solo imprime)
template <typename U>
void PrintElement(string& el, U multiplier) {
    cout << el << " ";
}

// ============ Demo BinaryTree (BST) ============
template <typename T>
void DemoBinaryTree() {
    cout << "=== Probar Binary Tree con tipo: " << typeid(T).name() << " ===" << endl;
    BinaryTree<AscendingBSTrait<T>> miBST;
    
    miBST.insert(T(5)); miBST.insert(T(3)); miBST.insert(T(7));
    miBST.insert(T(1)); miBST.insert(T(4)); miBST.insert(T(6)); miBST.insert(T(8));
    
    cout << "BST Inorder Fwd (Range-based for): ";
    for (auto val : miBST) cout << val << " ";
    cout << endl;

    cout << "BST Inorder Bwd:  ";
    for (auto it = miBST.rbegin(); it != miBST.rend(); ++it) cout << *it << " ";
    cout << endl;

    cout << "BST Preorder Fwd: ";
    for (auto it = miBST.preorder_begin(); it != miBST.preorder_end(); ++it) cout << *it << " ";
    cout << endl;

    cout << "BST Preorder Bwd: ";
    for (auto it = miBST.preorder_rbegin(); it != miBST.preorder_rend(); ++it) cout << *it << " ";
    cout << endl;

    cout << "BST Postorder Fwd: ";
    for (auto it = miBST.postorder_begin(); it != miBST.postorder_end(); ++it) cout << *it << " ";
    cout << endl;

    cout << "BST Postorder Bwd: ";
    for (auto it = miBST.postorder_rbegin(); it != miBST.postorder_rend(); ++it) cout << *it << " ";
    cout << "\n\n";
}

// ============ Test Adicionales BST ============
template <typename T>
void TestAdicionalesBST() {
    cout << "=== Test Adicionales BST con tipo: " << typeid(T).name() << " ===" << endl;
    BinaryTree<AscendingBSTrait<T>> t;
    t.insert(T(5)); t.insert(T(3)); t.insert(T(7)); t.insert(T(1)); t.insert(T(4));
    
    cout << "Original: " << t.ToString() << endl;
    cout << "ForEach (*2): ";
    t.ForEach(PrintElement<T, int>, 2);
    cout << endl;

    cout << "Borrando el 3 (su sucesor inorder debe reemplazarlo)...\n";
    t.remove(T(3));
    cout << "Post-remove(3): " << t.ToString() << endl;

    // Test Move semantics y Persistencia
    ofstream out("tree.txt");
    out << t;
    out.close();

    BinaryTree<AscendingBSTrait<T>> t2;
    ifstream in("tree.txt");
    in >> t2;
    in.close();
    cout << "Leido desde archivo via >> : " << t2.ToString() << endl;

    // Constructor copia
    BinaryTree<AscendingBSTrait<T>> t_copia = t2;
    cout << "Constructor copia (t_copia desde t2): " << t_copia.ToString() << endl;

    // Move constructor
    BinaryTree<AscendingBSTrait<T>> t3 = std::move(t2);
    cout << "t3 (Move Ctor desde t2): " << t3.ToString() << endl;
    cout << "t2 post-move (debe estar vacio): " << t2.ToString() << endl;

    // Operador de asignación por copia
    BinaryTree<AscendingBSTrait<T>> t4;
    t4 = t3;
    cout << "Operador= copia (t4 = t3): " << t4.ToString() << endl;

    // Operador de asignación por movimiento
    BinaryTree<AscendingBSTrait<T>> t5;
    t5 = std::move(t3);
    cout << "Operador= move (t5 = move(t3)): " << t5.ToString() << endl;
    cout << "t3 post-move: " << t3.ToString() << "\n\n";
}

#endif // __BINARYTREEDEMO_H__