//containers/ListsDemo.cpp
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <thread>

#include "../types.h"
#include "linkedlist.h"
#include "doublelinkedlist.h"
#include "circularlinkedlist.h"
#include "circulardoublelinkedlist.h"

using namespace std;

// prueba generica: inserta 5 datos, graba a archivo y lee de vuelta
template <typename Container>
void DemoList(Container& list, string fileName){
    list.insert(28, 15);
    list.insert(17, 25);
    list.insert(8, 35);
    list.insert(4, 45);
    list.insert(35, 55);
    cout << "  original:      " << list << endl;
    ofstream os(fileName);
    os << list << endl;
    os.close();
    Container listFromFile;
    ifstream  is(fileName);
    is >> listFromFile;
    cout << "  leido archivo: " << listFromFile << endl;
}

// linkedlist
void LinkedListDemo()
{
    cout << "[linkedlist]" << endl;

    // insercion ordenada + operator<<
    LinkedList<AscendingLinkedListTrait<T1>> list;
    list.insert(3,30); list.insert(1,10); list.insert(5,50);
    list.insert(2,20); list.insert(4,40);
    cout << "  insert      : " << list << endl;

    // operator>> + operator[]
    stringstream ss("[(10,100),(20,200),(30,300)]");
    ss >> list;
    cout << "  operator>>  : " << list << "  [0]=" << list[0] << " [2]=" << list[2] << endl;

    // move + pop
    LinkedList<AscendingLinkedListTrait<T1>> moved(std::move(list));
    auto [d,r] = moved.pop_front();
    cout << "  move+pop    : (" << d << "," << r << ") size orig=" << list.size() << endl;

    // persistencia archivo
    DemoList(moved, "AscLL.txt");
    cout << endl;
}

// doublelinkedlist
void DoubleLinkedListDemo()
{
    cout << "[doublelinkedlist]" << endl;

    DoubleLinkedList<AscendingDLLTrait<T1>> list;
    list.push_back(20,2); list.push_back(30,3);
    list.push_front(10,1); list.push_front(5,0);
    cout << "  push        : " << list << endl;

    auto [d1,r1] = list.pop_front();
    auto [d2,r2] = list.pop_back();
    cout << "  pop f/b     : (" << d1 << "," << r1 << ") (" << d2 << "," << r2 << ")" << endl;

    list.insert(3,30); list.insert(1,10); list.insert(2,20);
    cout << "  insert      : " << list << endl;

    // forward y backward
    cout << "  fwd: ";
    for (auto &v : list) cout << v << " ";
    cout << "| bwd: ";
    for (auto it = list.rbegin(); it != list.rend(); ++it) cout << *it << " ";
    cout << endl;

    // reverseforeach
    cout << "  revforeach  : ";
    list.ReverseForEach([](T1 &v){ cout << v << " "; });
    cout << endl;

    // copy + move
    DoubleLinkedList<AscendingDLLTrait<T1>> copied(list);
    DoubleLinkedList<AscendingDLLTrait<T1>> moved(std::move(copied));
    cout << "  copy+move   : " << moved << " orig=" << copied.size() << endl;

    cout << "  archivo     : ";
    DemoList(list, "AscDLL.txt");
    cout << endl;
}

// circularlinkedlist
void CircularLinkedListDemo()
{
    cout << "[circularlinkedlist]" << endl;

    CircularLinkedList<AscendingCLLTrait<T1>> list;
    list.insert(3,30); list.insert(1,10); list.insert(2,20);
    cout << "  insert      : " << list << endl;

    // circularforeach x2 vueltas
    cout << "  x2 vueltas  : ";
    list.circularForEach(2, [](T1 &v){ cout << v << " "; });
    cout << endl;

    // ranged-for da 1 vuelta exacta
    cout << "  fwd loop    : ";
    for (auto &v : list) cout << v << " ";
    cout << endl;

    // copy + circular en copia
    CircularLinkedList<AscendingCLLTrait<T1>> copied(list);
    cout << "  copy x2     : ";
    copied.circularForEach(2, [](T1 &v){ cout << v << " "; });
    cout << endl;

    cout << "  archivo     : ";
    DemoList(list, "AscCLL.txt");
    cout << endl;
}

// circulardoublelinkedlist
void CircularDoubleLinkedListDemo()
{
    cout << "[circulardoublelinkedlist]" << endl;

    CircularDoubleLinkedList<AscendingCDLLTrait<T1>> list;
    list.insert(3,30); list.insert(1,10); list.insert(2,20);
    cout << "  insert      : " << list << endl;

    // circularforeach con direccion
    cout << "  fwd x2      : ";
    list.circularForEach(2,  1, [](T1 &v){ cout << v << " "; });
    cout << endl;
    cout << "  bwd x2      : ";
    list.circularForEach(2, -1, [](T1 &v){ cout << v << " "; });
    cout << endl;

    // forward y backward ranged-for
    cout << "  fwd: ";
    for (auto &v : list) cout << v << " ";
    cout << "| bwd: ";
    for (auto it = list.rbegin(); it != list.rend(); ++it) cout << *it << " ";
    cout << endl;

    // reverseforeach
    cout << "  revforeach  : ";
    list.ReverseForEach([](T1 &v){ cout << v << " "; });
    cout << endl;

    // copy + move + circular en copia
    CircularDoubleLinkedList<AscendingCDLLTrait<T1>> copied(list);
    CircularDoubleLinkedList<AscendingCDLLTrait<T1>> moved(std::move(list));
    cout << "  move size=  : " << moved.size() << " orig=" << list.size() << endl;
    cout << "  copy bwd x2 : ";
    copied.circularForEach(2, -1, [](T1 &v){ cout << v << " "; });
    cout << endl;

    cout << "  archivo     : ";
    DemoList(copied, "AscCDLL.txt");
    cout << endl;
}

// concurrencia compartida, prueba 
template <typename Container>
void TestConcurrencia(const string& nombre){
    Container list;
    auto worker = [&list](int id){
        for (int i = 0; i < 1000; i++) list.push_front(i, id);
    };
    thread t1(worker,1), t2(worker,2), t3(worker,3), t4(worker,4), t5(worker,5);
    t1.join(); t2.join(); t3.join(); t4.join(); t5.join();
    cout << "  " << nombre << ": " << list.size()
         << " -> " << (list.size()==5000 ? "ok" : "fallo") << endl;
}

void ListsDemo()
{
    //LinkedListDemo();
    DoubleLinkedListDemo();
    CircularLinkedListDemo();
    CircularDoubleLinkedListDemo();

    cout << "[concurrencia]" << endl;
    //TestConcurrencia<LinkedList<AscendingLinkedListTrait<T1>>>("linkedlist");
    TestConcurrencia<DoubleLinkedList<AscendingDLLTrait<T1>>>("doublelinkedlist");
    TestConcurrencia<CircularLinkedList<AscendingCLLTrait<T1>>>("circularlinkedlist");
    TestConcurrencia<CircularDoubleLinkedList<AscendingCDLLTrait<T1>>>("circulardoublelinkedlist");

    cout << endl << "fin de las pruebas." << endl;
}