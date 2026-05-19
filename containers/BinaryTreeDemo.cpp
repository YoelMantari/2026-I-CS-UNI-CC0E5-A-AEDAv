#include <iostream>
#include "../BinaryTree.h"
#include "../containers/traits.h"

using namespace std;

void DemoBinaryTree() {
    cout << "Probar binary tree e itradores " << endl;
    BinaryTree<AscendingBSTrait<int>> miBST;
    miBST.insert(5); miBST.insert(3); miBST.insert(7);
    miBST.insert(1); miBST.insert(4); miBST.insert(6); miBST.insert(8);
    
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

void PrintElement(int& el, int multiplier) {
    cout << (el * multiplier) << " ";
}

void TestAdicionalesBST() {
    BinaryTree<AscendingBSTrait<int>> t;
    t.insert(5); t.insert(3); t.insert(7); t.insert(1); t.insert(4);
    
    cout << "Original: " << t.ToString() << endl;
    cout << "ForEach (*2): ";
    t.ForEach(PrintElement, 2);
    cout << endl;

    cout << "Borrando el 3 (su sucesor inorder debe reemplazarlo)...\n";
    t.remove(3);
    cout << "Post-remove(3): " << t.ToString() << endl;

    // Test Move semantics y Persistencia
    ofstream out("tree.txt");
    out << t;
    out.close();

    BinaryTree<AscendingBSTrait<int>> t2;
    ifstream in("tree.txt");
    in >> t2;
    in.close();
    cout << "Leido desde archivo via >> : " << t2.ToString() << endl;

    BinaryTree<AscendingBSTrait<int>> t3 = std::move(t2);
    cout << "t3 (Move Ctor desde t2): " << t3.ToString() << endl;
    cout << "t2 post-move (debe estar vacio): " << t2.ToString() << endl;
}
