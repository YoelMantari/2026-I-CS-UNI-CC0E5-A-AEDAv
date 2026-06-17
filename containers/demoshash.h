#ifndef __DEMOSHASH_H__
#define __DEMOSHASH_H__

#include <iostream>
#include <string>
#include <typeinfo>
#include <vector>

#include "hashtable.h"

using namespace std;

template <typename T>
vector<T> getHashTestData() {
    return {T(5), T(3), T(7), T(1), T(4), T(6), T(8)};
}

template <>
vector<string> getHashTestData<string>() {
    return {"hola", "abc", "mundo", "zzz", "foo", "bar", "xyz"};
}

template <typename T>
void DemoHashTable() {
    cout << "=== Probar HashTable" << typeid(T).name() << " ===" << endl;
    HashTable<HashTrait<T, Ref>> table;

    Ref ref = 10;
    for (const auto& key : getHashTestData<T>()) {
        table[key] = ref;
        ref += 10;
    }

    auto firstKey = getHashTestData<T>().front();
    table[firstKey] += 5;

    cout << "HashTable: " << table << endl;
    cout << "Size: " << table.size() << endl;
    cout << "Contiene primera llave: " << (table.contains(firstKey) ? "si" : "no") << endl;
    cout << "Ref primera llave: " << table.at(firstKey) << endl;

    cout << "Iteracion: ";
    for (auto item : table) {
        auto [key, ref] = item;
        cout << key << ":" << ref << " ";
    }
    cout << "\n";

    cout << "Size final: " << table.size() << "\n\n";
}

#endif // __DEMOSHASH_H__
