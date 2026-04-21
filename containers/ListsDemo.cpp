#include <iostream>
#include <sstream>
#include <thread>
#include <vector>
#include <utility>
#include "linkedlist.h"

using namespace std;

static void PrintElem(int &x, ostream &os, const string &sep){
    os << x << sep;
}

void ListsDemo(){
    using List = LinkedList<AscendingLinkedListTrait<int>>;

    cout << "t1 t2 t3" << endl;
    {
        List base;
        base.push_back(10, 100);
        base.push_back(20, 200);
        List copy(base);
        List moved(std::move(copy));
        cout << moved << endl;
    }

    cout << "t5 t6 t7 t8" << endl;
    List list;
    list.push_front(2, 20);
    list.push_front(1, 10);
    list.push_back(3, 30);
    list.push_back(4, 40);
    cout << list << endl;
    list.pop_front();
    list.pop_back();
    cout << list << endl;

    cout << "t4" << endl;
    for(auto it = list.begin(); it != list.end(); ++it)
        cout << *it << " ";
    cout << endl;

    cout << "t9" << endl;
    cout << list[0] << endl;

    cout << "t11" << endl;
    cout << list << endl;

    cout << "t10" << endl;
    istringstream iss("3 7 70 8 80 9 90");
    List from_stream;
    iss >> from_stream;
    cout << from_stream << endl;

    cout << "t12" << endl;
    ostringstream oss;
    from_stream.ForEach(PrintElem, oss, string("-"));
    cout << oss.str() << endl;

    cout << "t13" << endl;
    List conc;
    vector<thread> threads;
    for(int i = 0; i < 4; ++i){
        threads.emplace_back([&conc, i](){
            for(int j = 0; j < 5; ++j)
                conc.push_back(i * 10 + j, i * 10 + j);
        });
    }
    for(auto &t : threads)
        t.join();
    cout << conc.size() << endl;
}
