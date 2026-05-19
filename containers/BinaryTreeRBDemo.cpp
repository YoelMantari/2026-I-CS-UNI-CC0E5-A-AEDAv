#include <iostream>
#include "../BinaryTreeRB.h"
#include "../containers/traits.h"

using namespace std;

void DemoRBT() {
    cout << "Probar red black tree" << endl;
    BinaryTreeRB<AscendingRBTrait<int>> miRBT;
    for (int i = 1; i <= 7; ++i) {
        miRBT.insert(i);
    }
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
