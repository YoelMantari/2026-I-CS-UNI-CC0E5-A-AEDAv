#include <iostream>
#include "../BinaryTreeAVL.h"
#include "../containers/traits.h"

using namespace std;

void DemoAVL() {
    cout << "=== PRUEBA DE AVL TREE ===" << endl;
    BinaryTreeAVL<AscendingAVLTrait<int>> miAVL;
    for (int i = 1; i <= 7; ++i) {
        miAVL.insert(i);
    }
    
    cout << "AVL Inorder Fwd:  ";
    for (auto it = miAVL.begin(); it != miAVL.end(); ++it) cout << *it << " ";
    cout << endl;

    cout << "AVL Inorder Bwd:  ";
    for (auto it = miAVL.rbegin(); it != miAVL.rend(); ++it) cout << *it << " ";
    cout << endl;

    cout << "AVL Preorder Fwd: ";
    for (auto it = miAVL.preorder_begin(); it != miAVL.preorder_end(); ++it) cout << *it << " ";
    cout << endl;

    cout << "AVL Postorder Fwd: ";
    for (auto it = miAVL.postorder_begin(); it != miAVL.postorder_end(); ++it) cout << *it << " ";
    cout << "\n\n";
}
