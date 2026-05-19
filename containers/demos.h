#ifndef __DEMOS_H__
#define __DEMOS_H__

#include <iostream>
#include <fstream>
#include <typeinfo>
#include <vector>
#include "../BinaryTree.h"
#include "../BinaryTreeAVL.h"
#include "../BinaryTreeRB.h"
#include "traits.h"

using namespace std;

// ==================== DATOS DE PRUEBA POR TIPO ====================
template <typename T>
vector<T> getTestData() {
    return {T(5), T(3), T(7), T(1), T(4), T(6), T(8)};
}

// Especialización para string
template <>
vector<string> getTestData<string>() {
    return {"hola", "abc", "mundo", "zzz", "foo", "bar", "xyz"};
}

template <typename T>
vector<T> getSeqData(int n) {
    vector<T> v;
    for (int i = 1; i <= n; ++i) v.push_back(T(i));
    return v;
}

// Especialización para string: usa el número convertido
template <>
vector<string> getSeqData<string>(int n) {
    vector<string> v;
    for (int i = 1; i <= n; ++i) v.push_back(to_string(i));
    return v;
}

// ==================== AUXILIAR ====================
template <typename T, typename U>
void PrintElement(T& el, U multiplier) {
    cout << (el * multiplier) << " ";
}

template <typename U>
void PrintElement(string& el, U multiplier) {
    cout << el << " ";
}

// ==================== DEMO BST ====================
template <typename T>
void DemoBinaryTree() {
    cout << "=== Probar Binary Tree con tipo: " << typeid(T).name() << " ===" << endl;
    BinaryTree<AscendingBSTrait<T>> miBST;
    
    auto datos = getTestData<T>();
    for (auto& d : datos) miBST.insert(d);
    
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

// ==================== TEST ADICIONALES BST ====================
template <typename T>
void TestAdicionalesBST() {
    cout << "=== Test Adicionales BST con tipo: " << typeid(T).name() << " ===" << endl;
    BinaryTree<AscendingBSTrait<T>> t;
    
    t.insert(T(5)); t.insert(T(3)); t.insert(T(7)); t.insert(T(1)); t.insert(T(4));
    
    cout << "Original: " << t.ToString() << endl;
    cout << "ForEach (*2): ";
    t.ForEach(PrintElement<T, int>, 2);
    cout << endl;

    cout << "Borrando el 3...\n";
    t.remove(T(3));
    cout << "Post-remove(3): " << t.ToString() << endl;

    ofstream out("tree.txt");
    out << t;
    out.close();

    BinaryTree<AscendingBSTrait<T>> t2;
    ifstream in("tree.txt");
    in >> t2;
    in.close();
    cout << "Leido desde archivo: " << t2.ToString() << endl;

    BinaryTree<AscendingBSTrait<T>> t_copia = t2;
    cout << "Constructor copia: " << t_copia.ToString() << endl;

    BinaryTree<AscendingBSTrait<T>> t3 = std::move(t2);
    cout << "Move constructor: " << t3.ToString() << endl;
    cout << "t2 post-move: " << t2.ToString() << "\n\n";
}

// ==================== DEMO AVL ====================
template <typename T>
void DemoAVL() {
    cout << "=== Probar AVL Tree con tipo: " << typeid(T).name() << " ===" << endl;
    BinaryTreeAVL<AscendingAVLTrait<T>> miAVL;
    
    auto datos = getSeqData<T>(7);
    for (auto& d : datos) miAVL.insert(d);
    
    cout << "AVL Inorder Fwd (Range-based for): ";
    for (auto val : miAVL) cout << val << " ";
    cout << endl;

    cout << "AVL Inorder Bwd: ";
    for (auto it = miAVL.rbegin(); it != miAVL.rend(); ++it) cout << *it << " ";
    cout << endl;

    cout << "AVL Preorder Fwd: ";
    for (auto it = miAVL.preorder_begin(); it != miAVL.preorder_end(); ++it) cout << *it << " ";
    cout << endl;

    cout << "AVL Postorder Fwd: ";
    for (auto it = miAVL.postorder_begin(); it != miAVL.postorder_end(); ++it) cout << *it << " ";
    cout << "\n\n";
}

// ==================== DEMO RBT ====================
template <typename T>
void DemoRBT() {
    cout << "=== Probar Red Black Tree con tipo: " << typeid(T).name() << " ===" << endl;
    BinaryTreeRB<AscendingRBTrait<T>> miRBT;
    
    auto datos = getSeqData<T>(7);
    for (auto& d : datos) miRBT.insert(d);
    
    cout << "RBT Inorder Fwd (Range-based for): ";
    for (auto val : miRBT) cout << val << " ";
    cout << endl;

    cout << "RBT Preorder Fwd: ";
    for (auto it = miRBT.preorder_begin(); it != miRBT.preorder_end(); ++it) cout << *it << " ";
    cout << endl;

    cout << "RBT Postorder Fwd: ";
    for (auto it = miRBT.postorder_begin(); it != miRBT.postorder_end(); ++it) cout << *it << " ";
    cout << "\n\n";
}

#endif // __DEMOS_H__