#ifndef LRU_CACHE_H_
#define LRU_CACHE_H_

#include <cstdint>
#include <list>
#include <unordered_map>

template <typename K, typename V>
class LRUCache
{
    using it_type = typename std::list<K>::iterator;

    std::list<K> order;
    std::unordered_map<K, it_type> order_iterators;
    std::unordered_map<K, V> values;

    void erase_key(K key)
    {
        if (values.count(key) == 0)
        {
            return;
        }
        values.erase(key);
        order.erase(order_iterators[key]);
        order_iterators.erase(key);
    }

public:
    LRUCache() = default;
    LRUCache(const LRUCache &) = delete;

    uint32_t size() const
    {
        return values.size();
    }

    bool has(const K &key) const
    {
        return values.find(key) != values.end();
    }

    const K &back_key() const
    {
        return order.back();
    }

    const V &back_value() const
    {
        return values.find(order.back())->second;
    }

    const V &operator[](const K &key)
    {
        const auto &order_it = order_iterators[key];
        if (order_it != order.begin())
        {
            order.splice(order.begin(), order, order_it);
            order_iterators[key] = order.begin();
        }

        return values.find(key)->second;
    }

    void put(const K &key, V value)
    {
        erase_key(key);
        values[key] = std::move(value);
        order.push_front(key);
        order_iterators[key] = order.begin();
    }

    void pop()
    {
        erase_key(back_key());
    }
};

#endif
