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
    cout << "=== Probar HashTable con tipo llave: " << typeid(T).name() << " ===" << endl;
    HashTable<T, int> table;

    int value = 10;
    for (const auto& key : getHashTestData<T>()) {
        table.insert(key, value);
        value += 10;
    }

    auto firstKey = getHashTestData<T>().front();
    table[firstKey] += 5;

    cout << "HashTable: " << table << endl;
    cout << "Size: " << table.size() << endl;
    cout << "Contiene primera llave: " << (table.contains(firstKey) ? "si" : "no") << endl;
    cout << "Valor primera llave: " << table.search(firstKey) << endl;

    cout << "Iteracion: ";
    for (auto pair : table) {
        cout << pair.key << ":" << pair.value << " ";
    }
    cout << "\n";

    table.remove(firstKey);
    cout << "Luego de borrar primera llave: " << table << endl;
    cout << "Size final: " << table.size() << "\n\n";
}

#endif // __DEMOSHASH_H__
