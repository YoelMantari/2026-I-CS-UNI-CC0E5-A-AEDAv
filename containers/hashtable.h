#ifndef __HASHTABLE_H__
#define __HASHTABLE_H__

#include <cstddef>
#include <functional>
#include <iostream>
#include <mutex>
#include <shared_mutex>
#include <stdexcept>
#include <utility>

#include "../BinaryTreeAVL.h"
#include "../types.h"
#include "traits.h"

template <typename Key, typename Value, typename Comp = std::less<Key>>
struct HashTrait {
    // define los tipos que usara la tabla
    using KeyType = Key;
    using ValueType = Value;
    using CompType = Comp;
};

template <typename Key, typename Value, typename Comp = std::less<Key>>
struct KVPair {
    using value_type = KVPair;

    // par llave valor que se guarda dentro del avl
    Key key;
    mutable Value value;

    KVPair() : key(), value() {}
    KVPair(const Key& k, const Value& v = Value{})
        : key(k), value(v) {}

    bool operator<(const KVPair& other) const {
        return Comp{}(key, other.key);
    }

    bool operator>(const KVPair& other) const {
        return Comp{}(other.key, key);
    }

    bool operator==(const KVPair& other) const {
        return !(*this < other) && !(other < *this);
    }

    bool operator!=(const KVPair& other) const {
        return !(*this == other);
    }

    friend std::ostream& operator<<(std::ostream& os, const KVPair& pair) {
        return os << pair.key << ":" << pair.value;
    }

    // lee un par llave valor
    friend std::istream& operator>>(std::istream& is, KVPair& pair) {
        Key key{};
        Value value{};
        Token sep{};
        if (is >> key >> sep >> value && sep == ':')
            pair = KVPair{key, value};
        else
            is.setstate(std::ios_base::failbit);
        return is;
    }
};


template <typename Pair>
// adapta el par para reutilizar el avl como estructura interna
struct HashTreeTrait : public BaseTrait<Pair, std::less<Pair>, AVLNode<Pair>> {};

template <typename Trait>
class HashTable {
public:
    using Key = typename Trait::KeyType;
    using Value = typename Trait::ValueType;
    using Comp = typename Trait::CompType;
    using Pair = KVPair<Key, Value, Comp>;
    using Tree = BinaryTreeAVL<HashTreeTrait<Pair>>;
    using Node = AVLNode<Pair>;

private:
    // arbol avl donde se almacenan los pares
    Tree m_tree;
    mutable std::shared_mutex m_mtx;

    // busca por llave creando un par temporal
    Node* buscar(const Key& key) const {
        return m_tree.find_node(Pair{key});
    }

public:
    HashTable() = default;

    // constructor copia
    HashTable(const HashTable& other) {
        std::shared_lock<std::shared_mutex> lock(other.m_mtx);
        m_tree = other.m_tree;
    }

    // move constructor
    HashTable(HashTable&& other) noexcept {
        std::unique_lock<std::shared_mutex> lock(other.m_mtx);
        m_tree = std::move(other.m_tree);
    }

    HashTable& operator=(const HashTable& other) {
        if (this != &other) {
            std::unique_lock<std::shared_mutex> lockSelf(m_mtx, std::defer_lock);
            std::shared_lock<std::shared_mutex> lockOther(other.m_mtx, std::defer_lock);
            std::lock(lockSelf, lockOther);
            m_tree = other.m_tree;
        }
        return *this;
    }

    HashTable& operator=(HashTable&& other) noexcept {
        if (this != &other) {
            std::unique_lock<std::shared_mutex> lockSelf(m_mtx, std::defer_lock);
            std::unique_lock<std::shared_mutex> lockOther(other.m_mtx, std::defer_lock);
            std::lock(lockSelf, lockOther);
            m_tree = std::move(other.m_tree);
        }
        return *this;
    }

    ~HashTable() = default;

    //operador []
    Value& operator[](const Key& key) {
        std::unique_lock<std::shared_mutex> lock(m_mtx);
        if (Node* found = buscar(key))
            return found->m_data.value;

        m_tree.insert(Pair{key});
        return buscar(key)->m_data.value;
    }

    const Value& at(const Key& key) const {
        std::shared_lock<std::shared_mutex> lock(m_mtx);
        Node* found = buscar(key);
        if (!found)
            throw std::out_of_range("key no existe");
        return found->m_data.value;
    }

    bool contains(const Key& key) const {
        std::shared_lock<std::shared_mutex> lock(m_mtx);
        return buscar(key) != nullptr;
    }

    std::size_t size() const { return m_tree.size(); }
    bool isEmpty() const { return size() == 0; }

    // permite recorrer con for const auto referencia key value en m
    auto begin() { return m_tree.begin(); }
    auto end() { return m_tree.end(); }
    auto begin() const { return const_cast<Tree&>(m_tree).begin(); }
    auto end() const { return const_cast<Tree&>(m_tree).end(); }

    // operator salida
    friend std::ostream& operator<<(std::ostream& os, const HashTable& table) {
        std::shared_lock<std::shared_mutex> lock(table.m_mtx);
        os << "{";
        bool first = true;
        for (auto it = const_cast<Tree&>(table.m_tree).begin();
             it != const_cast<Tree&>(table.m_tree).end(); ++it) {
            if (!first) os << ", ";
            os << *it;
            first = false;
        }
        return os << "}";
    }

    // operator entrada
    friend std::istream& operator>>(std::istream& is, HashTable& table) {
        Token open{};
        if (!(is >> open) || open != '{') {
            is.setstate(std::ios_base::failbit);
            return is;
        }

        table = HashTable{};
        is >> std::ws;
        if (is.peek() == '}') {
            is.get();
            return is;
        }

        while (is) {
            Pair pair{};
            if (!(is >> pair))
                return is;

            // inserta cada par usando operator corchetes
            table[pair.key] = pair.value;

            is >> std::ws;
            Token next = static_cast<Token>(is.peek());
            if (next == ',') {
                is.get();
                continue;
            }
            if (next == '}') {
                is.get();
                return is;
            }

            is.setstate(std::ios_base::failbit);
            return is;
        }

        return is;
    }
};

#endif // __HASHTABLE_H__
