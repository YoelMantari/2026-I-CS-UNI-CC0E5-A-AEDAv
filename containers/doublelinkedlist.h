#ifndef __DOUBLELINKEDLIST_H__
#define __DOUBLELINKEDLIST_H__

#include "linkedlist.h"

// TODO Los iteradores ahora son forward y backward
// Crear 2 nuevos i
// nodo de lista doble enlanzada, extender LLNode con puntero del antrior
template <typename T>
class DLLNode : public LLNode<T, DLLNode<T>>{
    using Node = DLLNode<T>;
    private:
        Node *m_pPrev;
    public:
        DLLNode() : LLNode<T, DLLNode<T>>(), m_pPrev(nullptr) {}
        DLLNode(T data, Ref ref, Node *next = nullptr, Node *prev = nullptr) : LLNode<T, DLLNode<T>>(data, ref, next), m_pPrev(prev) {}

        Node*  getPrev() const     { return m_pPrev; }
        void   setPrev(Node *prev) { m_pPrev = prev; }
        Node*& getPrevRef()        { return m_pPrev; }

};

// ordenar de mayor a menor ascending trait
template <typename T>
struct AscendingDLLTrait : BaseTrait<T, less<T>, DLLNode<T>> {};

// ordenar de mayor a menor descendng trait
template <typename T>
struct DescendingDLLTrait : BaseTrait<T, greater<T>, DLLNode<T>> {};

// forward iterator, recorremos la lista doble de inicio a final
template <typename Container>
class DLLForwardIterator : public general_iterator<Container, DLLForwardIterator<Container>>{
public:
    using MySelf = DLLForwardIterator<Container>;
    using Parent = general_iterator<Container, MySelf>;
    using Parent::Parent;
    
    // operador ++ para avanzar al siguiente nido con getNext
    MySelf operator++(){
        if (this->m_pNode) this->m_pNode = this->m_pNode->getNext();
        return *this;
    }
};

//backward iteratr, recorrer lista de final a incio 
template <typename Container>
class DLLBackwardIterator : public general_iterator<Container, DLLBackwardIterator<Container>>{
public:
    using MySelf = DLLBackwardIterator<Container>;
    using Parent = general_iterator<Container, MySelf>;
    using Parent::Parent;
    
    // operator++ backward, retroceder al nodo anterior con getPrev
    MySelf operator++(){
        if (this->m_pNode) this->m_pNode = this->m_pNode->getPrev();
        return *this;
    }
};

template <typename Trait>
class DoubleLinkedList : public LinkedList<Trait>{
public:
    using value_type      = typename Trait::value_type;
    using Node            = typename Trait::Node;
    using Comp            = typename Trait::Comp;
    using MySelf          = DoubleLinkedList<Trait>;
    using forward_iterator  = DLLForwardIterator<MySelf>;
    using backward_iterator = DLLBackwardIterator<MySelf>;
    friend forward_iterator;
    friend backward_iterator;

private:
    Node   *m_pRoot = nullptr;
    Node   *m_tail  = nullptr;
    size_t  m_size  = 0;
    Comp    m_comp;
    mutable shared_mutex m_mtx;

    //internal insert, insersion ordenada actual y prev
    void internal_insert(Node *&actual, Node *pPrevNode, const value_type &value, Ref ref){
        if (!actual || m_comp(value, actual->getDataRef())){
            Node *newNode = new Node(value, ref, actual, pPrevNode);
            if (actual)
                actual->setPrev(newNode);
            actual = newNode;
            m_size++;
            if (newNode->getNext() == nullptr)
                m_tail = newNode;
            return;
        }
        internal_insert(actual->getNextRef(), actual, value, ref);
    }

public:
    DoubleLinkedList() : LinkedList<Trait>() {}

    // TODO: Copy constructor
    //       Simplificar y abstraer el bucle de copia de Nodes
    //       Es posible que no necesites este constructor ya que lo heredaste
    //copy constructor, copia nodo a nodo con pb
    DoubleLinkedList(const DoubleLinkedList &other)
        : LinkedList<Trait>(), m_pRoot(nullptr), m_tail(nullptr), m_size(0)
    {
        shared_lock<shared_mutex> lock(other.m_mtx);
        for (Node *c = other.m_pRoot; c != nullptr; c = c->getNext())
            push_back(c->getData(), c->getRef());
    }

    // TODO: Move constructor
    //move constructor, tomar los punteros y el tamanio del orignen
    DoubleLinkedList(DoubleLinkedList &&other)
        : LinkedList<Trait>(), m_pRoot(nullptr), m_tail(nullptr), m_size(0)
    {
        unique_lock<shared_mutex> lock(other.m_mtx);
        m_pRoot = std::exchange(other.m_pRoot, nullptr);
        m_tail  = std::exchange(other.m_tail,  nullptr);
        m_size  = std::exchange(other.m_size,  0);
    }

    // TODO: Copy assignment operator
    //copy assignment, limpiar liusta actual y copia nodo a nodo
    DoubleLinkedList &operator=(const DoubleLinkedList &other){
        if (this != &other){
            clear();
            shared_lock<shared_mutex> lock(other.m_mtx);
            for (Node *c = other.m_pRoot; c != nullptr; c = c->getNext())
                push_back(c->getData(), c->getRef());
        }
        return *this;
    }

    // TODO: Move assignment operator
    //move assignment, limpiar y tomar los recursos del origen
    DoubleLinkedList &operator=(DoubleLinkedList &&other){
        if (this != &other){
            clear();
            unique_lock<shared_mutex> lock(other.m_mtx);
            m_pRoot = std::exchange(other.m_pRoot, nullptr);
            m_tail  = std::exchange(other.m_tail,  nullptr);
            m_size  = std::exchange(other.m_size,  0);
        }
        return *this;
    }

    //destructor, liberar los nodos
    virtual ~DoubleLinkedList() { clear(); }

    // lmpiar hacia adelante
    void clear(){ 
        unique_lock<shared_mutex> lock(m_mtx);
        Node *act = m_pRoot;
        while (act){
            Node *next = act->getNext();
            delete act;
            act = next;
        }
        m_pRoot = nullptr;
        m_tail  = nullptr;
        m_size  = 0;
    }

    //push front, insertar de inicio y enlazar prev del antrior root
    void push_front(value_type value, Ref ref) override {
        unique_lock<shared_mutex> lock(m_mtx);
        Node *newNode = new Node(value, ref, m_pRoot, nullptr);
        if (m_pRoot) m_pRoot->setPrev(newNode);
        else         m_tail = newNode;
        m_pRoot = newNode;
        m_size++;
    }

    //push back, insertar al final y enlaza al next del ultmo tail
    void push_back(value_type value, Ref ref) override {
        unique_lock<shared_mutex> lock(m_mtx);
        Node *newNode = new Node(value, ref, nullptr, m_tail);
        if (m_tail) m_tail->setNext(newNode);
        else        m_pRoot = newNode;
        m_tail = newNode;
        m_size++;
    }

    //insert, insetrar manteniendo ambos enlaces
    void insert(const value_type &value, Ref ref) override {
        unique_lock<shared_mutex> lock(m_mtx);
        internal_insert(m_pRoot, nullptr, value, ref);
        if (m_size == 1) m_tail = m_pRoot;
    }

    //pop front, eliminar el primer nodo y el prev del nuevo root
    std::tuple<value_type, Ref> pop_front() override {
        unique_lock<shared_mutex> lock(m_mtx);
        if(!m_pRoot) throw runtime_error("lista vacia");
        Node *temp   = m_pRoot;
        auto  result = std::make_tuple(temp->getData(), temp->getRef());
        m_pRoot      = temp->getNext();
        if(m_pRoot) m_pRoot->setPrev(nullptr);
        else        m_tail = nullptr;
        delete temp;
        m_size--;
        return result;
    }

    //pop back, eliminar el ultimo nodo y 
    std::tuple<value_type, Ref> pop_back() override {
        unique_lock<shared_mutex> lock(m_mtx);
        if (!m_pRoot) throw runtime_error("lista vacia");
        Node *temp   = m_tail;
        auto  result = std::make_tuple(temp->getData(), temp->getRef());
        m_tail       = temp->getPrev();
        if (m_tail) m_tail->setNext(nullptr);
        else        m_pRoot = nullptr;
        delete temp;
        m_size--;
        return result;
    }

    // acceso por indice con lectura concurrente
    value_type &operator[](size_t index) override {
        shared_lock<shared_mutex> lock(m_mtx);
        if (index >= m_size) throw out_of_range("fuera de rango");
        Node *act = m_pRoot;
        for (size_t i = 0; i < index; ++i) act = act->getNext();
        return act->getDataRef();
    }

    // retornar cantidad de elemntos
    size_t size() const override {
        shared_lock<shared_mutex> lock(m_mtx);
        return m_size;
    }

    //iteradores
    forward_iterator  begin()  { return forward_iterator (this, m_pRoot); }
    forward_iterator  end()    { return forward_iterator (this, nullptr); }
    backward_iterator rbegin() { return backward_iterator(this, m_tail);  }
    backward_iterator rend()   { return backward_iterator(this, nullptr); }

    //foreach, aplicar funcion a cada elmento hacia adelante
    template <typename Func, typename... Args>
    void ForEach(Func func, Args &&...args){
        unique_lock<shared_mutex> lock(m_mtx);
        if (m_size == 0) return;
        for (auto &item : *this)
            func(item, std::forward<Args>(args)...);
    }

    //reverse foreach, aplicar funcion a cada elmento hacia atras
    template <typename Func, typename... Args>
    void ReverseForEach(Func func, Args &&...args){
        unique_lock<shared_mutex> lock(m_mtx);
        if (m_size == 0) return;
        for (auto it = rbegin(); it != rend(); ++it)
            func(*it, std::forward<Args>(args)...);
    }

    // operator<<, mostrar forward y backward para ver los enlaces
    friend ostream &operator<<(ostream &os, const DoubleLinkedList &list){
        shared_lock<shared_mutex> lock(list.m_mtx);
        os << "[";
        Node* act = list.m_pRoot;
        while(act){
            os << "(" << act->getData() << "," << act->getRef() << ")";
            if (act->getNext()) os << ",";
            act = act->getNext();
        }
        os << "]";
        os << " |bwd:[";
        act = list.m_tail;
        while (act){
            os << "(" << act->getData() << "," << act->getRef() << ")";
            if (act->getPrev()) os << ",";
            act = act->getPrev();
        }
        os << "]";
        return os;
    }

    //operator>> leer formatos [(val, ref)]
    friend istream &operator>>(istream &is, DoubleLinkedList &list){
        char ch;
        if (!(is >> ch) || ch != '['){
            is.clear(ios_base::failbit);
            return is;
        }
        value_type val;
        Ref        ref;
        char       comma, parenClose;
        while (is >> ch && ch != ']'){
            if (ch == '(')
                if (is >> val >> comma >> ref >> parenClose)
                    if (comma == ',' && parenClose == ')')
                        list.insert(val, ref);
        }
        is.ignore(numeric_limits<streamsize>::max(), '\n');
        return is;
    }
};

#endif