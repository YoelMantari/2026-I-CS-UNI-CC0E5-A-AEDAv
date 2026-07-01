//BTreeDemo.cpp
//#include <iostream.h>
#include <iostream>
#include <sstream>

#include "../types.h"
#include "BTree.h"

//const char * keys="CDAMPIWNBKEHOLJYQZFXVRTSGU";
BTreeText keys1 = "D1XJ2xTg8zKL9AhijOPQcEowRSp0NbW567BUfCqrs4FdtYZakHIuvGV3eMylmn";
BTreeText keys2 = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

const BTreeOrder BTreeSize = 3;

template <typename Tree>
void InsertKeys(Tree& tree, BTreeText keys)
{
       for (Size i = 0; keys[i]; i++)
       {
              tree.Insert(keys[i], static_cast<typename Tree::ref_type>(i * i));
       }
}

template <typename Tree>
void SearchKeys(Tree& tree, BTreeText keys, std::ostream& os)
{
       for (Size i = 0; keys[i]; i++)
       {
              typename Tree::ref_type ref = tree.Search(keys[i]);
              if( ref != typename Tree::ref_type(-1) )
                     os << "Encontrado " << keys[i] << " ref = " << ref << '\n';
              else
                     os << "No encontrado " << keys[i] << '\n';
       }
}

template <typename Tree>
void ShowForEach(Tree& tree, std::ostream& os)
{
       os << "\nRecorrido directo con ForEach variadic:\n";
       tree.ForEach(
              [](const auto& node, BTreeLevel level, std::ostream& out)
              {
                     out << "nivel " << level << ": "
                         << node << '\n';
              },
              os);
}

template <typename Tree>
void ShowFirstThat(Tree& tree, BTreeKey key, std::ostream& os)
{
       auto* found = tree.FirstThat(
              [](const auto& node, BTreeLevel, BTreeKey keyToFind)
              {
                     return node.key == keyToFind;
              },
              key);

       if( found )
              os << "\nFirstThat variadic encontro: " << *found << '\n';
       else
              os << "\nFirstThat variadic no encontro la clave buscada\n";
}

template <typename Tree>
void ShowForwardIterator(Tree& tree, std::ostream& os)
{
       os << "\nRecorrido con ForwardIterator:\n";

       for (auto it = tree.begin(); it != tree.end(); ++it)
       {
              os << *it << '\n';
       }
}

template <typename Tree>
void ShowBackwardIterator(Tree& tree, std::ostream& os)
{
       os << "\nRecorrido con BackwardIterator:\n";

       for (auto it = tree.rbegin(); it != tree.rend(); ++it)
       {
              os << *it << '\n';
       }
}

void ShowNodeStreamOperators(std::ostream& os)
{
       os << "\nPrueba operator>> y operator<< en BTreeNode:\n";

       using node_type = BTreeNode<BTreeKey, BTreeRef>;

       std::istringstream input("(Q,361)");
       node_type node{};

       input >> node;

       if (input)
              os << "Nodo leido: " << node << '\n';
       else
              os << "Error al leer nodo\n";
}

template <typename Traits>
void ShowTreeInputOperator(std::ostream& os)
{
       os << "\nPrueba operator>> en BTree:\n";

       BTree<Traits> tree(BTreeSize);

       std::istringstream input("(A,0) (B,4) (C,9)");
       input >> tree;

       os << tree;
}

template <typename Traits>
void Demo(BTreeText insertKeys, BTreeText searchKeys, BTreeKey firstThatKey, std::ostream& os)
{
       BTree<Traits> tree(BTreeSize);

       InsertKeys(tree, insertKeys);

       os << "B-Tree despues de insertar claves:\n";
       os << tree;

       os << "\nBusquedas:\n";
       SearchKeys(tree, searchKeys, os);

       ShowForEach(tree, os);
       ShowFirstThat(tree, firstThatKey, os);
       ShowForwardIterator(tree, os);
       ShowBackwardIterator(tree, os);
       ShowTreeInputOperator<Traits>(os);
       ShowNodeStreamOperators(os);
}

/*const char * keys="CDAMPIWNBKEHOLJYQZFXVRTSGU";
const char * keys2="CDAMPIWNBKEHOLJYQZFXVRTSGU";
const int BTreeSize = 3;
main (int argc, char * argv)
*/

void BTreeDemo()
{
       using DemoTraits = BTreeTraits<BTreeKey, BTreeRef>;

       Demo<DemoTraits>(keys1, keys2, 'Q', std::cout);
}
