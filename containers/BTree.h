// btree.h

#ifndef BTREE_H
#define BTREE_H

#include <algorithm>
#include <cstddef>
#include <iostream>
#include <iterator>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <utility>
#include <vector>

#include "../types.h"
#include "BTreePage.h"
#include "general_iterator.h"

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

       class ForwardIterator : public general_iterator<node_type>
       {
       public:
              using Base = general_iterator<node_type>;

              using iterator_category = std::forward_iterator_tag;
              using value_type = typename Base::value_type;
              using difference_type = typename Base::difference_type;
              using pointer = typename Base::pointer;
              using reference = typename Base::reference;

              ForwardIterator() = default;

              ForwardIterator(std::shared_ptr<std::vector<node_type>> data, Size pos)
                     : Base(std::move(data), pos)
              {
              }

              ForwardIterator& operator++()
              {
                     this->Advance();
                     return *this;
              }

              ForwardIterator operator++(int)
              {
                     ForwardIterator tmp = *this;
                     ++(*this);
                     return tmp;
              }
       };

       class BackwardIterator : public general_iterator<node_type>
       {
       public:
              using Base = general_iterator<node_type>;

              using iterator_category = std::forward_iterator_tag;
              using value_type = typename Base::value_type;
              using difference_type = typename Base::difference_type;
              using pointer = typename Base::pointer;
              using reference = typename Base::reference;

              BackwardIterator() = default;

              BackwardIterator(std::shared_ptr<std::vector<node_type>> data, Size pos)
                     : Base(std::move(data), pos)
              {
              }

              BackwardIterator& operator++()
              {
                     this->Advance();
                     return *this;
              }

              BackwardIterator operator++(int)
              {
                     BackwardIterator tmp = *this;
                     ++(*this);
                     return tmp;
              }
       };

       using iterator = ForwardIterator;
       using const_iterator = ForwardIterator;
       using reverse_iterator = BackwardIterator;
       using const_reverse_iterator = BackwardIterator;

public:
       BTree(std::size_t order = DEFAULT_BTREE_ORDER, Bool unique = true);
       ~BTree();
       //int           Open (char * name, int mode);
       //int           Create (char * name, int mode);
       //int           Close ();
       Bool            Insert (const value_type& key, const ref_type& ref);
       Bool            Remove (const value_type& key, const ref_type& ref);
       ref_type        Search (const value_type& key);
       std::size_t     size() const;
       std::size_t     height() const;
       std::size_t     GetOrder() const;

       void            Print (std::ostream &os);

       template <typename Func, typename... Args>
       void ForEach(Func&& func, Args&&... args)
       {
              std::shared_lock<std::shared_mutex> lock(m_mtx);
              m_Root.ForEach(std::forward<Func>(func), 0, std::forward<Args>(args)...);
       }

       template <typename Predicate, typename... Args>
       node_type* FirstThat(Predicate&& pred, Args&&... args)
       {
              std::shared_lock<std::shared_mutex> lock(m_mtx);
              node_type* found = m_Root.FirstThat(std::forward<Predicate>(pred), 0, std::forward<Args>(args)...);
              if( !found )
                     return nullptr;

              static thread_local node_type foundCopy{};
              foundCopy = *found;
              return &foundCopy;
       }

       iterator begin()
       {
              auto snapshot = std::make_shared<std::vector<node_type>>();

              ForEach(
                     [snapshot](const node_type& node, BTreeLevel) -> void
                     {
                            snapshot->push_back(node);
                     });

              return iterator(snapshot, Size{});
       }

       iterator end()
       {
              return iterator{};
       }

       reverse_iterator rbegin()
       {
              auto snapshot = std::make_shared<std::vector<node_type>>();

              ForEach(
                     [snapshot](const node_type& node, BTreeLevel) -> void
                     {
                            snapshot->push_back(node);
                     });

              std::reverse(snapshot->begin(), snapshot->end());

              return reverse_iterator(snapshot, Size{});
       }

       reverse_iterator rend()
       {
              return reverse_iterator{};
       }

protected:
       page_type       m_Root;
       std::size_t     m_Height;  // height of tree
       std::size_t     m_Order;   // order of tree
       std::size_t     m_NumKeys; // number of keys
       Bool            m_Unique;  // Accept the elements only once ?
       mutable std::shared_mutex m_mtx;
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
std::size_t BTree<Traits>::size() const
{
       std::shared_lock<std::shared_mutex> lock(m_mtx);
       return m_NumKeys;
}

template <typename Traits>
std::size_t BTree<Traits>::height() const
{
       std::shared_lock<std::shared_mutex> lock(m_mtx);
       return m_Height;
}

template <typename Traits>
std::size_t BTree<Traits>::GetOrder() const
{
       std::shared_lock<std::shared_mutex> lock(m_mtx);
       return m_Order;
}

template <typename Traits>
Bool BTree<Traits>::Insert(const value_type& key, const ref_type& ref)
{
       bt_ErrorCode error = bt_ok;
       {
              std::shared_lock<std::shared_mutex> lock(m_mtx);
              error = m_Root.Insert(key, ref);
       }
       if( error == bt_duplicate )
               return false;

       std::unique_lock<std::shared_mutex> lock(m_mtx);
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
       std::unique_lock<std::shared_mutex> lock(m_mtx);
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
       std::unique_lock<std::shared_mutex> lock(m_mtx);
       ref_type ref = ref_type(-1);
       m_Root.Search(key, ref);
       return ref;
}

template <typename Traits>
void BTree<Traits>::Print(std::ostream &os)
{
       std::shared_lock<std::shared_mutex> lock(m_mtx);
       m_Root.Print(os);
}

template <typename Traits>
std::ostream& operator<<(std::ostream& os, BTree<Traits>& tree)
{
       tree.Print(os);
       return os;
}

template <typename Traits>
std::istream& operator>>(std::istream& is, BTree<Traits>& tree)
{
       using node_type = typename BTree<Traits>::node_type;

       while (true)
       {
              node_type node{};

              if (!(is >> node))
              {
                     if (is.eof())
                     {
                            is.clear();
                     }

                     break;
              }

              tree.Insert(node.key, node.ref);
       }

       return is;
}

#endif
