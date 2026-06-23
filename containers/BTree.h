// btree.h

#ifndef BTREE_H
#define BTREE_H

#include <cstddef>
#include <iostream>
#include <utility>

#include "../types.h"
#include "BTreePage.h"

#define DEFAULT_BTREE_ORDER 3

template <typename Traits>
class BTree
// this is the full version of the BTree
{
public:
       using value_type = typename Traits::value_type;
       using ref_type   = typename Traits::ref_type;
       using node_type  = typename Traits::node_type;
       using page_type  = CBTreePage<Traits>;
       using ObjectInfo = node_type;

public:
       BTree(std::size_t order = DEFAULT_BTREE_ORDER, Bool unique = true);
       ~BTree();
       //int           Open (char * name, int mode);
       //int           Create (char * name, int mode);
       //int           Close ();
       Bool            Insert (const value_type& key, const ref_type& ref);
       Bool            Remove (const value_type& key, const ref_type& ref);
       ref_type        Search (const value_type& key);
       std::size_t     size() const  { return m_NumKeys; }
       std::size_t     height() const { return m_Height; }
       std::size_t     GetOrder() const { return m_Order; }

       void            Print (std::ostream &os);

       template <typename Func, typename... Args>
       void ForEach(Func&& func, Args&&... args)
       {
              m_Root.ForEach(std::forward<Func>(func), 0, std::forward<Args>(args)...);
       }

       template <typename Predicate, typename... Args>
       node_type* FirstThat(Predicate&& pred, Args&&... args)
       {
              return m_Root.FirstThat(std::forward<Predicate>(pred), 0, std::forward<Args>(args)...);
       }

protected:
       page_type       m_Root;
       std::size_t     m_Height;  // height of tree
       std::size_t     m_Order;   // order of tree
       std::size_t     m_NumKeys; // number of keys
       Bool            m_Unique;  // Accept the elements only once ?
};

template <typename Traits>
BTree<Traits>::BTree(std::size_t order, Bool unique)
                               : m_Root(2 * order  + 1, unique),
                                 m_Height(1),
                                 m_Order(order),
                                 m_NumKeys(0),
                                 m_Unique(unique)
{
       m_Root.SetMaxKeysForChilds(order);
}

template <typename Traits>
BTree<Traits>::~BTree()
{
}

template <typename Traits>
Bool BTree<Traits>::Insert(const value_type& key, const ref_type& ref)
{
       bt_ErrorCode error = m_Root.Insert(key, ref);
       if( error == bt_duplicate )
               return false;
       m_NumKeys++;
       if( error == bt_overflow )
       {
               m_Root.SplitRoot();
               m_Height++;
       }
       return true;
}

template <typename Traits>
Bool BTree<Traits>::Remove (const value_type& key, const ref_type& ref)
{
       bt_ErrorCode error = m_Root.Remove(key, ref);
       if( error == bt_nofound )
               return false;
       m_NumKeys--;

       if( error == bt_rootmerged )
               m_Height--;
       return true;
}

template <typename Traits>
typename BTree<Traits>::ref_type BTree<Traits>::Search (const value_type& key)
{
       ref_type ref = ref_type(-1);
       m_Root.Search(key, ref);
       return ref;
}

template <typename Traits>
void BTree<Traits>::Print(std::ostream &os)
{
       m_Root.Print(os);
}

template <typename Traits>
std::ostream& operator<<(std::ostream& os, BTree<Traits>& tree)
{
       tree.Print(os);
       return os;
}

#endif
