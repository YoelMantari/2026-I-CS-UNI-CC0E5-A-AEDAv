//traits.h
#ifndef __TRAITS_H__
#define __TRAITS_H__
#include <cstddef>
#include <functional> // para less y greater
#include <istream>
#include <ostream>

#include "../types.h"

template <typename _Node, typename _Comp>
struct BaseTrait{
    using Node         = _Node;
    using node_type    = _Node;
    using value_type   = typename _Node::value_type;
    using ref_type     = typename _Node::ref_type;
    using Comp         = _Comp;
    using compare_type = _Comp;
};

template <typename _Node>
struct AscendingTrait : public BaseTrait<_Node, std::less<typename _Node::value_type>>{
};
template <typename _Node>
struct DescendingTrait : public BaseTrait<_Node, std::greater<typename _Node::value_type>>{
};

template <typename Key, typename Ref = long>
struct BTreeNode
{
    using value_type = Key;
    using ref_type = Ref;

    value_type key{};
    ref_type ref{};
    std::size_t use_counter{0};

    BTreeNode() = default;

    BTreeNode(const value_type& k, const ref_type& r)
        : key(k), ref(r), use_counter(0)
    {
    }

    operator value_type() const { return key; }
    std::size_t GetUseCounter() const { return use_counter; }
};

template <typename Key, typename Ref>
std::ostream& operator<<(std::ostream& os, const BTreeNode<Key, Ref>& node)
{
    return os << "(" << node.key << "," << node.ref << ")";
}

template <typename Key, typename Ref>
std::istream& operator>>(std::istream& is, BTreeNode<Key, Ref>& node)
{
    using node_type = BTreeNode<Key, Ref>;
    using value_type = typename node_type::value_type;
    using ref_type = typename node_type::ref_type;

    BTreeKey open{};
    BTreeKey comma{};
    BTreeKey close{};
    value_type key{};
    ref_type ref{};

    is >> open >> key >> comma >> ref >> close;

    if (!is) {
        return is;
    }

    const BTreeKey expectedOpen = static_cast<BTreeKey>('(');
    const BTreeKey expectedComma = static_cast<BTreeKey>(',');
    const BTreeKey expectedClose = static_cast<BTreeKey>(')');

    if (open != expectedOpen || comma != expectedComma || close != expectedClose) {
        is.setstate(std::ios::failbit);
        return is;
    }

    node.key = key;
    node.ref = ref;
    node.use_counter = 0;

    return is;
}

template <typename Key, typename Ref = long>
struct BTreeTraits : public BaseTrait<BTreeNode<Key, Ref>, std::less<Key>>
{
};

#endif // __TRAITS_H__
