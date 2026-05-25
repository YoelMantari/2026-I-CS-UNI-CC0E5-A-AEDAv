#ifndef __DEMOS_H__
#define __DEMOS_H__

#include <fstream>
#include <initializer_list>
#include <iostream>
#include <thread>
#include "heap.h"
#include "../types.h"

using MinHeap = Heap<MinHeapTrait<T1>>;
using MaxHeap = Heap<MaxHeapTrait<T1>>;

static const std::initializer_list<T1> kTestValues = {10, 4, 15, 1, 7, 12, 3};

template <typename HeapType>
inline void PrintHeapValues(const HeapType& heap){
    heap.forEach([](const typename HeapType::Node& node){
        std::cout << node.getData() << " ";
    });
}

inline void DemoMinHeap(){
    std::cout << "\nMinHeap" << std::endl;
    MinHeap h;

    std::cout << "insert:" << std::endl;
    for(T1 v : kTestValues){
        h.insert(v, v * 10);
        std::cout << "insert(" << v << ")\n" << h.toString();
    }

    auto p = h.peek();
    std::cout << "peek(minimo): " << p.getData() << " ref:" << p.getRef() << std::endl;

    {
        std::ofstream os("minheap.txt");
        os << h;
    }

    std::cout << "operator<<:\n" << h << std::endl;

    MinHeap h2;
    {
        std::ifstream is("minheap.txt");
        is >> h2;
    }
    std::cout << "operator>> leido:\n" << h2 << std::endl;

    std::cout << "forEach:  ";
    PrintHeapValues(h);
    std::cout << std::endl;

    std::cout << "\nextract (orden ascendente):" << std::endl;
    while(!h.isEmpty()){
        auto node = h.extract();
        std::cout << "  extract -> " << node.getData() << " | ";
        PrintHeapValues(h);
        std::cout << std::endl;
    }
}

inline void DemoMaxHeap(){
    std::cout << "\nMaxHeap" << std::endl;
    MaxHeap h;

    std::cout << "insert:" << std::endl;
    for(T1 v : kTestValues){
        h.insert(v, v * 10);
        std::cout << "insert(" << v << ")\n" << h.toString();
    }

    auto p = h.peek();
    std::cout << "peek(maximo): " << p.getData() << " ref:" << p.getRef() << std::endl;

    std::cout << "forEach:  ";
    PrintHeapValues(h);
    std::cout << std::endl;

    std::cout << "\nextract (orden descendente):" << std::endl;
    while(!h.isEmpty()){
        auto node = h.extract();
        std::cout << "  extract -> " << node.getData() << " | ";
        PrintHeapValues(h);
        std::cout << std::endl;
    }
}

inline void DemoHeapConcurrencia(){
    std::cout << "\nconcurrencia" << std::endl;
    MinHeap h;

    auto worker = [&h](MinHeap::value_type id){
        MinHeap::value_type i{};
        while(i < 200){
            h.insert(i * id, id);
            ++i;
        }
    };

    std::thread th1(worker, 1), th2(worker, 2), th3(worker, 3), th4(worker, 4), th5(worker, 5);
    th1.join();
    th2.join();
    th3.join();
    th4.join();
    th5.join();

    std::cout << "size 1000: " << h.size() << std::endl;
    auto p = h.peek();
    std::cout << "peek minimo: " << p.getData() << std::endl;
}

inline void HeapDemo(){
    std::cout << std::endl;
    std::cout << "PRUEBAS HEAP" << std::endl;
    DemoMinHeap();
    DemoMaxHeap();
    DemoHeapConcurrencia();
    std::cout << "\nFIN DE LAS PRUEBAS" << std::endl;
}

inline void DemoHeaps(){
    HeapDemo();
}

#endif // __DEMOS_H__
