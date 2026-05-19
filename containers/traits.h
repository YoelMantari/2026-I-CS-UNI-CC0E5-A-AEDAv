#ifndef __TRAITS_H__
#define __TRAITS_H__
#include <functional> // para std::less y std::greater

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

// Accesos directos convenientes y más semánticos
template <typename T> class BinaryTreeNode;
template <typename T> class AVLNode;

template <typename T>
using AscendingBSTrait = AscendingTrait<BinaryTreeNode<T>>;

template <typename T>
using DescendingBSTrait = DescendingTrait<BinaryTreeNode<T>>;

template <typename T>
using AscendingAVLTrait = AscendingTrait<AVLNode<T>>;

template <typename T>
using DescendingAVLTrait = DescendingTrait<AVLNode<T>>;

#endif // __TRAITS_H__

template <typename T> class RBTreeNode;

template <typename T>
using AscendingRBTrait = AscendingTrait<RBTreeNode<T>>;

template <typename T>
using DescendingRBTrait = DescendingTrait<RBTreeNode<T>>;
