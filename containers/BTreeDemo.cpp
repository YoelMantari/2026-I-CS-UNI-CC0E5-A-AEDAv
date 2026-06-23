//BTreeDemo.cpp
//#include <iostream.h>
#include <iostream>

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
                         << node.key << "->" << node.ref << '\n';
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
              os << "\nFirstThat variadic encontro: " << found->key << "->" << found->ref << '\n';
       else
              os << "\nFirstThat variadic no encontro la clave buscada\n";
}

template <typename Traits>
void Demo(BTreeText insertKeys, BTreeText searchKeys, BTreeKey firstThatKey, std::ostream& os)
{
       BTree<Traits> tree(BTreeSize);

       InsertKeys(tree, insertKeys);

       os << "B-Tree despues de insertar claves:\n";
       tree.Print(os);

       os << "\nBusquedas:\n";
       SearchKeys(tree, searchKeys, os);

       ShowForEach(tree, os);
       ShowFirstThat(tree, firstThatKey, os);
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
