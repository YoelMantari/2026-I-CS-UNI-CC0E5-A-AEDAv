#include <iostream>
#include "containers/demos.h"

using namespace std;

int main() {
    cout << "Iniciando bateria de pruebas de los tres Arboles (CRTP)...\n\n";
    
    DemoBinaryTree();
    TestAdicionalesBST();
    cout << "\n";
    DemoAVL();
    DemoRBT();

    cout << "Todas las pruebas finalizaron con exito." << endl;
    return 0;
}
