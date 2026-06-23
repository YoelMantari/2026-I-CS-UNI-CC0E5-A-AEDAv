#include <iostream>

#include "BTree.h"

const char * keys1 = "D1XJ2xTg8zKL9AhijOPQcEowRSp0NbW567BUfCqrs4FdtYZakHIuvGV3eMylmn";
const char * keys2 = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

const std::size_t BTreeSize = 3;

template <typename Tree>
void InsertKeys(Tree& tree, const typename Tree::value_type* keys)
{
       using ref_type = typename Tree::ref_type;

       for (std::size_t i = 0; keys[i]; i++)
       {
              tree.Insert(keys[i], static_cast<ref_type>(i * i));
       }
}

template <typename Tree>
void PrintSearchResults(Tree& tree, const typename Tree::value_type* keys, std::ostream& os)
{
       for (std::size_t i = 0; keys[i]; i++)
       {
              typename Tree::ref_type ref = tree.Search(keys[i]);
              if( ref != typename Tree::ref_type(-1) )
                     os << "Encontrado " << keys[i] << " ref = " << ref << '\n';
              else
                     os << "No encontrado " << keys[i] << '\n';
       }
}

template <typename Tree>
void PrintWithForEach(Tree& tree, std::ostream& os)
{
       tree.ForEach(
              [](const auto& node, std::size_t level, std::ostream& out)
              {
                     out << "nivel " << level << ": "
                         << node.key << "->" << node.ref << '\n';
              },
              os);
}

template <typename Tree>
void PrintFirstThat(Tree& tree, const typename Tree::value_type& key, std::ostream& os)
{
       auto* found = tree.FirstThat(
              [](const auto& node, std::size_t, const typename Tree::value_type& keyToFind)
              {
                     return node.key == keyToFind;
              },
              key);

       if( found )
              os << "\nFirstThat encontro: " << found->key << "->" << found->ref << '\n';
       else
              os << "\nFirstThat no encontro la clave buscada\n";
}

template <typename Traits>
void Demo(const typename Traits::value_type* insertKeys,
          const typename Traits::value_type* searchKeys,
          const typename Traits::value_type& firstThatKey,
          std::ostream& os)
{
       BTree<Traits> tree(BTreeSize);

       InsertKeys(tree, insertKeys);

       os << "B-Tree despues de insertar claves:\n";
       tree.Print(os);

       os << "\nBusquedas:\n";
       PrintSearchResults(tree, searchKeys, os);

       os << "\nRecorrido directo con ForEach:\n";
       PrintWithForEach(tree, os);

       PrintFirstThat(tree, firstThatKey, os);
}

void BTreeDemo()
{
       using DemoTraits = BTreeTraits<char, long>;

       Demo<DemoTraits>(keys1, keys2, 'Q', std::cout);
}
