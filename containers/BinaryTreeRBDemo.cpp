#ifndef __BINARYTREERBDEMO_H__
#define __BINARYTREERBDEMO_H__

#include <iostream>
#include <typeinfo>
#include "../BinaryTreeRB.h"
#include "../containers/traits.h"

using namespace std;

template <typename T>
void DemoRBT() {
    cout << "=== Probar Red Black Tree con tipo: " << typeid(T).name() << " ===" << endl;
    BinaryTreeRB<AscendingRBTrait<T>> miRBT;
    for (int i = 1; i <= 7; ++i) {
        miRBT.insert(T(i));
    }
    
    cout << "RBT Inorder Fwd (Range-based for): ";
    for (auto val : miRBT) cout << val << " ";
    cout << endl;

    cout << "RBT Inorder Bwd: ";
    for (auto it = miRBT.rbegin(); it != miRBT.rend(); ++it) cout << *it << " ";
    cout << endl;

    cout << "RBT Preorder Fwd: ";
    for (auto it = miRBT.preorder_begin(); it != miRBT.preorder_end(); ++it) cout << *it << " ";
    cout << endl;

    cout << "RBT Preorder Bwd: ";
    for (auto it = miRBT.preorder_rbegin(); it != miRBT.preorder_rend(); ++it) cout << *it << " ";
    cout << endl;

    cout << "RBT Postorder Fwd: ";
    for (auto it = miRBT.postorder_begin(); it != miRBT.postorder_end(); ++it) cout << *it << " ";
    cout << endl;

    cout << "RBT Postorder Bwd: ";
    for (auto it = miRBT.postorder_rbegin(); it != miRBT.postorder_rend(); ++it) cout << *it << " ";
    cout << endl;

    // Test Move semantics y Persistencia
    cout << "RBT ToString: " << miRBT.ToString() << endl;
    
    ofstream out("rbtree.txt");
    out << miRBT;
    out.close();

    BinaryTreeRB<AscendingRBTrait<T>> miRBT2;
    ifstream in("rbtree.txt");
    in >> miRBT2;
    in.close();
    cout << "RBT Leido desde archivo: " << miRBT2.ToString() << endl;

    // Constructor copia
    BinaryTreeRB<AscendingRBTrait<T>> miRBT3 = miRBT2;
    cout << "RBT Copia: " << miRBT3.ToString() << endl;

    // Move constructor
    BinaryTreeRB<AscendingRBTrait<T>> miRBT4 = std::move(miRBT2);
    cout << "RBT Move: " << miRBT4.ToString() << endl;
    cout << "RBT Original post-move (vacio): " << miRBT2.ToString() << "\n\n";
}

#endif // __BINARYTREERBDEMO_H__