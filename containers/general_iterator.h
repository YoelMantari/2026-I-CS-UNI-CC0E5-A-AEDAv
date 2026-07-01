// general_iterator.h
#ifndef __ITERATOR_H__
#define __ITERATOR_H__

#include <iterator>
#include <memory>
#include <utility>
#include <vector>

#include "../types.h"

template <typename Node>
class general_iterator
{
public:
    using node_type = Node;
    using container_type = std::vector<node_type>;

    using value_type = node_type;
    using difference_type = std::ptrdiff_t;
    using pointer = const node_type*;
    using reference = const node_type&;

    general_iterator() = default;

    general_iterator(std::shared_ptr<container_type> data, Size pos)
        : m_Data(std::move(data)), m_Pos(pos)
    {
    }

    reference operator*() const
    {
        return (*m_Data)[m_Pos];
    }

    pointer operator->() const
    {
        return &(*m_Data)[m_Pos];
    }

    Bool operator==(const general_iterator& other) const
    {
        if (IsEnd() && other.IsEnd())
        {
            return true;
        }

        return m_Data == other.m_Data && m_Pos == other.m_Pos;
    }

    Bool operator!=(const general_iterator& other) const
    {
        return !(*this == other);
    }

protected:
    Bool IsEnd() const
    {
        return !m_Data || m_Pos >= m_Data->size();
    }

    void Advance()
    {
        ++m_Pos;
    }

    std::shared_ptr<container_type> m_Data{};
    Size m_Pos{};
};

#endif
