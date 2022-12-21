#ifndef INDEXED_DEQUEUE_H_
#define INDEXED_DEQUEUE_H_

#include <memory>
#include <stdexcept>
#include <unordered_map>

template <typename T>
class IndexedDequeue
{
    std::unordered_map<int, T> items;
    int _start_index = 0;
    int _end_index = 0;

public:

    // First index
    int start_index() const
    {
        return _start_index;
    }

    // One index above last elem
    int end_index() const
    {
        return _end_index;
    }

    uint32_t size() const
    {
        return _end_index - _start_index;
    }

    const T &operator[](int index) const
    {
        auto it = items.find(index);
        if (it == items.end())
        {
            throw std::out_of_range("Invalid item index");
        }
        return it->second;
    }

    const T &back() const
    {
        return (*this)[_end_index - 1];
    }

    void prepend(T item)
    {
        items.emplace(--_start_index, std::move(item));
    }

    void append(T item)
    {
        items.emplace(_end_index++, std::move(item));
    }

    void clear()
    {
        items.clear();
        _start_index = 0;
        _end_index = 0;
    }
};

#endif
