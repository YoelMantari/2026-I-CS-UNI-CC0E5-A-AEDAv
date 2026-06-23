#ifndef __TRAITS_H__
#define __TRAITS_H__
#include <cstddef>
#include <functional> // para less y greater

template <typename _Node, typename _Comp>
struct BaseTrait{
    using Node       = _Node;
    using value_type = typename _Node::value_type;
    using Comp       = _Comp;
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

template <typename Key, typename Ref = long>
struct BTreeTraits
{
    using value_type = Key;
    using ref_type = Ref;
    using node_type = BTreeNode<Key, Ref>;
};

#endif // __TRAITS_H__
