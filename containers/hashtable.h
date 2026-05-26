#ifndef __HASHTABLE_H__
#define __HASHTABLE_H__

#include <cstddef>
#include <functional>
#include <iostream>
#include <mutex>
#include <shared_mutex>
#include <sstream>
#include <stdexcept>
#include <tuple>
#include <utility>

#include "../BinaryTreeAVL.h"
#include "../types.h"
#include "traits.h"

template <typename Key, typename Value>
struct HashEntry {
    using value_type = Key;

    Key key;
    Value value;
    Ref ref;

    HashEntry() : key{}, value{}, ref{} {}
    explicit HashEntry(const Key& k) : key(k), value{}, ref{} {}
    HashEntry(const Key& k, const Value& v, Ref r = Ref{})
        : key(k), value(v), ref(r) {}

    Key& getDataRef() { return key; }
    Key getData() const { return key; }

    friend bool operator==(const HashEntry& a, const HashEntry& b) {
        return a.key == b.key;
    }

    friend bool operator!=(const HashEntry& a, const HashEntry& b) {
        return !(a == b);
    }

    friend bool operator<(const HashEntry& a, const HashEntry& b) {
        return a.key < b.key;
    }

    friend bool operator>(const HashEntry& a, const HashEntry& b) {
        return b < a;
    }

    friend std::ostream& operator<<(std::ostream& os, const HashEntry& entry) {
        return os << entry.key << ":" << entry.value;
    }
};

template <typename Key, typename Value>
struct HashNode : public BinaryTreeNodeBase<HashNode<Key, Value>, HashEntry<Key, Value>> {
    using Base = BinaryTreeNodeBase<HashNode<Key, Value>, HashEntry<Key, Value>>;
    using value_type = typename Base::value_type;
    using height_t = typename Base::height_t;

    std::size_t m_height;

    HashNode() : Base(HashEntry<Key, Value>{}), m_height(1) {}
    explicit HashNode(HashEntry<Key, Value> data) : Base(data), m_height(1) {}
    HashNode(const Key& key, const Value& value, Ref ref = Ref{})
        : Base(HashEntry<Key, Value>{key, value, ref}), m_height(1) {}

    friend std::ostream& operator<<(std::ostream& os, const HashNode& node) {
        return os << node.m_data;
    }
};

template <typename Key, typename Value>
struct HashEntryComp {
    bool operator()(const HashEntry<Key, Value>& a,
                    const HashEntry<Key, Value>& b) const {
        return a.key < b.key;
    }
};

template <typename Key, typename Value>
struct HashBucketTrait {
    using Node = HashNode<Key, Value>;
    using value_type = HashEntry<Key, Value>;
    using Comp = HashEntryComp<Key, Value>;
};

template <typename Key, typename Value>
class HashBucket : public BinaryTreeAVL<HashBucketTrait<Key, Value>> {
public:
    using Base = BinaryTreeAVL<HashBucketTrait<Key, Value>>;
    using Entry = HashEntry<Key, Value>;
    using Node = HashNode<Key, Value>;

    Node* findEntry(const Key& key) const {
        return this->find_node(Entry{key});
    }

    bool insertOrAssign(const Key& key, const Value& value, Ref ref = Ref{}) {
        if (auto* node = findEntry(key)) {
            node->m_data.value = value;
            node->m_data.ref = ref;
            return false;
        }

        this->insert(Entry{key, value, ref});
        return true;
    }
};

template <typename Key, typename Value>
struct KVPair {
    const Key& key;
    Value& value;
};

namespace std {
    template <typename Key, typename Value>
    struct tuple_size<KVPair<Key, Value>> : integral_constant<size_t, 2> {};

    template <typename Key, typename Value>
    struct tuple_element<0, KVPair<Key, Value>> { using type = const Key; };

    template <typename Key, typename Value>
    struct tuple_element<1, KVPair<Key, Value>> { using type = Value; };
}

template <std::size_t I, typename Key, typename Value>
decltype(auto) get(KVPair<Key, Value>& pair) {
    if constexpr (I == 0) return pair.key;
    else return pair.value;
}

template <std::size_t I, typename Key, typename Value>
decltype(auto) get(const KVPair<Key, Value>& pair) {
    if constexpr (I == 0) return pair.key;
    else return pair.value;
}

template <typename Key, typename Value>
class HashTable {
public:
    using Entry = HashEntry<Key, Value>;
    using Node = HashNode<Key, Value>;
    using Bucket = HashBucket<Key, Value>;
    using key_type = Key;
    using mapped_type = Value;

private:
    static constexpr std::size_t kDefaultCapacity = 17;

    Bucket* m_buckets;
    std::size_t m_capacity;
    std::size_t m_size;
    mutable std::shared_mutex m_mtx;

    std::size_t normalized_capacity(std::size_t capacity) const {
        return capacity == 0 ? kDefaultCapacity : capacity;
    }

    std::size_t bucket_index(const Key& key) const {
        return std::hash<Key>{}(key) % m_capacity;
    }

    Node* find_node_unlocked(const Key& key) const {
        return m_buckets[bucket_index(key)].findEntry(key);
    }

public:
    struct Iterator {
        HashTable* table;
        std::size_t bucket;
        std::size_t pos;

        KVPair<Key, Value> operator*() const {
            auto it = table->m_buckets[bucket].begin();
            for (std::size_t i = 0; i < pos; ++i) ++it;
            auto* node = it.getNode();
            return {node->m_data.key, node->m_data.value};
        }

        Iterator& operator++() {
            if (bucket >= table->m_capacity) return *this;

            if (pos + 1 < table->m_buckets[bucket].size()) {
                ++pos;
                return *this;
            }

            ++bucket;
            pos = 0;
            while (bucket < table->m_capacity && table->m_buckets[bucket].size() == 0)
                ++bucket;
            return *this;
        }

        bool operator==(const Iterator& other) const {
            return table == other.table && bucket == other.bucket && pos == other.pos;
        }

        bool operator!=(const Iterator& other) const {
            return !(*this == other);
        }
    };

    explicit HashTable(std::size_t capacity = kDefaultCapacity)
        : m_buckets(nullptr),
          m_capacity(normalized_capacity(capacity)),
          m_size(0) {
        m_buckets = new Bucket[m_capacity];
    }

    HashTable(const HashTable& other)
        : m_buckets(nullptr), m_capacity(0), m_size(0) {
        std::shared_lock<std::shared_mutex> lock(other.m_mtx);
        m_capacity = other.m_capacity;
        m_size = other.m_size;
        m_buckets = new Bucket[m_capacity];
        for (std::size_t i = 0; i < m_capacity; ++i)
            m_buckets[i] = other.m_buckets[i];
    }

    HashTable(HashTable&& other) noexcept
        : m_buckets(nullptr), m_capacity(0), m_size(0) {
        std::unique_lock<std::shared_mutex> lock(other.m_mtx);
        m_capacity = std::exchange(other.m_capacity, 0);
        m_size = std::exchange(other.m_size, 0);
        m_buckets = std::exchange(other.m_buckets, nullptr);
    }

    HashTable& operator=(const HashTable& other) {
        if (this != &other) {
            std::unique_lock<std::shared_mutex> lock(m_mtx, std::defer_lock);
            std::shared_lock<std::shared_mutex> otherLock(other.m_mtx, std::defer_lock);
            std::lock(lock, otherLock);

            Bucket* newBuckets = new Bucket[other.m_capacity];
            for (std::size_t i = 0; i < other.m_capacity; ++i)
                newBuckets[i] = other.m_buckets[i];

            delete[] m_buckets;
            m_capacity = other.m_capacity;
            m_size = other.m_size;
            m_buckets = newBuckets;
        }
        return *this;
    }

    HashTable& operator=(HashTable&& other) noexcept {
        if (this != &other) {
            std::unique_lock<std::shared_mutex> lock(m_mtx, std::defer_lock);
            std::unique_lock<std::shared_mutex> otherLock(other.m_mtx, std::defer_lock);
            std::lock(lock, otherLock);

            delete[] m_buckets;
            m_capacity = std::exchange(other.m_capacity, 0);
            m_size = std::exchange(other.m_size, 0);
            m_buckets = std::exchange(other.m_buckets, nullptr);
        }
        return *this;
    }

    ~HashTable() {
        delete[] m_buckets;
    }

    Iterator begin() {
        for (std::size_t i = 0; i < m_capacity; ++i) {
            if (m_buckets[i].size() > 0)
                return Iterator{this, i, 0};
        }
        return end();
    }

    Iterator end() {
        return Iterator{this, m_capacity, 0};
    }

    void insert(const Key& key, const Value& value, Ref ref = Ref{}) {
        std::unique_lock<std::shared_mutex> lock(m_mtx);
        if (m_buckets[bucket_index(key)].insertOrAssign(key, value, ref))
            ++m_size;
    }

    void remove(const Key& key) {
        std::unique_lock<std::shared_mutex> lock(m_mtx);
        Bucket& bucket = m_buckets[bucket_index(key)];
        if (!bucket.findEntry(key)) return;
        bucket.remove(Entry{key});
        --m_size;
    }

    Value& operator[](const Key& key) {
        std::unique_lock<std::shared_mutex> lock(m_mtx);
        Bucket& bucket = m_buckets[bucket_index(key)];
        if (auto* node = bucket.findEntry(key))
            return node->m_data.value;

        bucket.insert(Entry{key, Value{}, Ref{}});
        ++m_size;
        return bucket.findEntry(key)->m_data.value;
    }

    Value& search(const Key& key) {
        std::shared_lock<std::shared_mutex> lock(m_mtx);
        if (auto* node = find_node_unlocked(key))
            return node->m_data.value;
        throw std::runtime_error("key no encontrada");
    }

    const Value& search(const Key& key) const {
        std::shared_lock<std::shared_mutex> lock(m_mtx);
        if (auto* node = find_node_unlocked(key))
            return node->m_data.value;
        throw std::runtime_error("key no encontrada");
    }

    bool contains(const Key& key) const {
        std::shared_lock<std::shared_mutex> lock(m_mtx);
        return find_node_unlocked(key) != nullptr;
    }

    std::size_t size() const {
        std::shared_lock<std::shared_mutex> lock(m_mtx);
        return m_size;
    }

    std::size_t capacity() const {
        std::shared_lock<std::shared_mutex> lock(m_mtx);
        return m_capacity;
    }

    bool isEmpty() const {
        return size() == 0;
    }

    void clear() {
        std::unique_lock<std::shared_mutex> lock(m_mtx);
        delete[] m_buckets;
        m_buckets = new Bucket[m_capacity];
        m_size = 0;
    }

    template <typename Func, typename... Args>
    void forEach(Func func, Args&&... args) {
        std::shared_lock<std::shared_mutex> lock(m_mtx);
        for (std::size_t i = 0; i < m_capacity; ++i) {
            m_buckets[i].ForEach([&](Entry& entry) {
                func(entry.key, entry.value, std::forward<Args>(args)...);
            });
        }
    }

    std::string toString() const {
        std::shared_lock<std::shared_mutex> lock(m_mtx);
        std::ostringstream oss;
        oss << "{";
        bool first = true;

        for (std::size_t i = 0; i < m_capacity; ++i) {
            const_cast<Bucket&>(m_buckets[i]).ForEach([&](Entry& entry) {
                if (!first) oss << ", ";
                oss << entry;
                first = false;
            });
        }

        oss << "}";
        return oss.str();
    }

    friend std::ostream& operator<<(std::ostream& os, const HashTable& table) {
        return os << table.toString();
    }

    friend std::istream& operator>>(std::istream& is, HashTable& table) {
        char ch;
        if (!(is >> ch) || ch != '{') {
            is.setstate(std::ios_base::failbit);
            return is;
        }

        table.clear();

        while (true) {
            is >> std::ws;
            if (is.peek() == '}') {
                is.get();
                break;
            }

            Key key{};
            Value value{};
            char sep{};
            if (!(is >> key >> sep >> value) || sep != ':') {
                is.setstate(std::ios_base::failbit);
                return is;
            }
            table.insert(key, value);

            is >> std::ws;
            if (is.peek() == ',') is.get();
        }

        return is;
    }
};

#endif // __HASHTABLE_H__
