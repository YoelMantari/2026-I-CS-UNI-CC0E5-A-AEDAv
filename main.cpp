#include <iostream>
#include <string>
#include "containers/demos.h"

using namespace std;

int main() {
    cout << "Iniciando bateria de pruebas de los tres Arboles (CRTP)...\n\n";
    
    // ========== int ==========
    cout << "========== TIPO: int ==========\n\n";
    DemoBinaryTree<int>();
    TestAdicionalesBST<int>();
    DemoAVL<int>();
    DemoRBT<int>();

    // ========== double ==========
    cout << "\n========== TIPO: double ==========\n\n";
    DemoBinaryTree<double>();
    TestAdicionalesBST<double>();
    DemoAVL<double>();
    DemoRBT<double>();

    // ========== string ==========
    cout << "\n========== TIPO: string ==========\n\n";
    DemoBinaryTree<string>();
    DemoAVL<string>();
    DemoRBT<string>();

    cout << "\nTodas las pruebas finalizaron con exito." << endl;
    return 0;
}