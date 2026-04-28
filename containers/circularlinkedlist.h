#ifndef __CircularLinkList_H__
#define __CircularLinkList_H__

#include "linkedlist.h"


template <typename T>
struct AscendingCLLTrait : BaseTrait<T, less<T>, LLNode<T>> {};

template <typename T>
struct DescendingCLLTrait : BaseTrait<T, greater<T>, LLNode<T>> {};

//forward iterator
template <typename Container>
class CLLForwardIterator: public general_iterator<Container, CLLForwardIterator<Container>>{
public:
    using MySelf = CLLForwardIterator<Container>;
    using Parent = general_iterator<Container, MySelf>;
    using Node   = typename Container::Node;
private:
    Node *m_start; // nodo inicial para ver el cierre del linkd circular
    bool  m_started; // evitar que se cancele al inicio con la primera comparacion
public:
    // constructor sentinel, devuele el iterador que ya termino
    CLLForwardIterator(Container *pContainer, Node *pNode): Parent(pContainer, pNode), m_start(pNode), m_started(false) {}
    CLLForwardIterator(Container *pContainer, Node *pNode, bool sentinel): Parent(pContainer, pNode), m_start(nullptr), m_started(sentinel) {}
    //operador++ forward, avanza y se anula al volver al inicio
    MySelf operator++(){
        if (this->m_pNode){
            m_started      = true;
            this->m_pNode  = this->m_pNode->getNext();
            if (this->m_pNode == m_start)
                this->m_pNode = nullptr;
        }
        return *this;
    }
};


// lista circlar
template <typename Trait>
class CircularLinkedList : public LinkedList<Trait>{
public:
    using value_type       = typename Trait::value_type;
    using Node             = typename Trait::Node;
    using Comp             = typename Trait::Comp;
    using MySelf           = CircularLinkedList<Trait>;
    using forward_iterator = CLLForwardIterator<MySelf>;
    friend forward_iterator;
private:
    Node  *m_pRoot = nullptr;
    Node  *m_tail  = nullptr;
    size_t m_size  = 0;
    Comp   m_comp;
    mutable shared_mutex m_mtx;
    //internal insert
    void internal_insert(const value_type &value, Ref ref){
        Node *newNode = new Node(value, ref);
        m_size++;
        // si la lista esta vacia apunta apinta asi mismo
        if (!m_pRoot){
            newNode->setNext(newNode);
            m_pRoot = newNode;
            m_tail  = newNode;
            return;
        }
        //el nuevo nodo se vuelve la cabeza. tail apunta hacia el nuevo root
        if (m_comp(value, m_pRoot->getDataRef())){
            newNode->setNext(m_pRoot);
            m_tail->setNext(newNode);
            m_pRoot = newNode;
            return;
        }

        // buscar el slot en medio o al final
        Node *act = m_pRoot;
        while (act->getNext() != m_pRoot && !m_comp(value, act->getNext()->getDataRef())){
            act = act->getNext();
        }
        // insertar newnode
        newNode->setNext(act->getNext());
        act->setNext(newNode);

        // nuevoi nodo para a ser tail
        if (act == m_tail)
            m_tail = newNode;
    }

public:
    //constructores
    CircularLinkedList() : LinkedList<Trait>() {}

    //copiar constructor
    CircularLinkedList(const CircularLinkedList &other): LinkedList<Trait>(), m_pRoot(nullptr), m_tail(nullptr), m_size(0){
        shared_lock<shared_mutex> lock(other.m_mtx);
        if (!other.m_pRoot) return;
        Node *curr = other.m_pRoot;
        do {
            push_back(curr->getData(), curr->getRef());
            curr = curr->getNext();
        } while (curr != other.m_pRoot);
    }

    //move constructor
    CircularLinkedList(CircularLinkedList &&other): LinkedList<Trait>(), m_pRoot(nullptr), m_tail(nullptr), m_size(0){
        unique_lock<shared_mutex> lock(other.m_mtx);
        m_pRoot = std::exchange(other.m_pRoot, nullptr);
        m_tail  = std::exchange(other.m_tail,  nullptr);
        m_size  = std::exchange(other.m_size,  0);
    }

    //copy assignament
    CircularLinkedList &operator=(const CircularLinkedList &other){
        if (this != &other){
            clear();
            shared_lock<shared_mutex> lock(other.m_mtx);
            if (!other.m_pRoot) return *this;
            Node *curr = other.m_pRoot;
            do {
                push_back(curr->getData(), curr->getRef());
                curr = curr->getNext();
            } while (curr != other.m_pRoot);
        }
        return *this;
    }

    //move assignment, limpiar y tomar
    CircularLinkedList &operator=(CircularLinkedList &&other){
        if (this != &other){
            clear();
            unique_lock<shared_mutex> lock(other.m_mtx);
            m_pRoot = std::exchange(other.m_pRoot, nullptr);
            m_tail  = std::exchange(other.m_tail,  nullptr);
            m_size  = std::exchange(other.m_size,  0);
        }
        return *this;
    }

    //destructor
    virtual ~CircularLinkedList() { clear(); }
    void clear(){
        unique_lock<shared_mutex> lock(m_mtx);
        if (!m_pRoot) return;
        m_tail->setNext(nullptr);
        Node *curr = m_pRoot;
        while (curr){
            Node *next = curr->getNext();
            delete curr;
            curr = next;
        }
        m_pRoot = nullptr;
        m_tail  = nullptr;
        m_size  = 0;
    }

    //push front
    void push_front(value_type value, Ref ref) override{
        unique_lock<shared_mutex> lock(m_mtx);
        Node *newNode = new Node(value, ref);
        if (!m_pRoot){
            newNode->setNext(newNode); // circulo de uno
            m_pRoot = newNode;
            m_tail  = newNode;
        }
        else{
            newNode->setNext(m_pRoot);
            m_tail->setNext(newNode); // cierre circular
            m_pRoot = newNode;
        }
        m_size++;
    }

    //push back
    void push_back(value_type value, Ref ref) override{
        unique_lock<shared_mutex> lock(m_mtx);
        Node *newNode = new Node(value, ref);
        if (!m_pRoot){
            newNode->setNext(newNode);
            m_pRoot = newNode;
            m_tail  = newNode;
        }
        else{
            newNode->setNext(m_pRoot);  // nuevo tail apunta al root
            m_tail->setNext(newNode);   // antiguo tail apunta al nuevo
            m_tail = newNode;
        }
        m_size++;
    }

    //insertar
    void insert(const value_type &value, Ref ref) override{
        unique_lock<shared_mutex> lock(m_mtx);
        internal_insert(value, ref);
    }

    //pop front
    std::tuple<value_type, Ref> pop_front() override{
        unique_lock<shared_mutex> lock(m_mtx);
        if (!m_pRoot) throw runtime_error("La lista esta vacia");
        Node *temp   = m_pRoot;
        auto  result = std::make_tuple(temp->getData(), temp->getRef());
        if (m_size == 1){
            m_pRoot = nullptr;
            m_tail  = nullptr;
        }
        else{
            m_pRoot = temp->getNext();
            m_tail->setNext(m_pRoot); // reparar cierre circular
        }
        delete temp;
        m_size--;
        return result;
    }

    //pop back
    std::tuple<value_type, Ref> pop_back() override{
        unique_lock<shared_mutex> lock(m_mtx);
        if (!m_pRoot) throw runtime_error("La lista esta vacia");
        auto result = std::make_tuple(m_tail->getData(), m_tail->getRef());
        if (m_size == 1){
            delete m_tail;
            m_pRoot = nullptr;
            m_tail  = nullptr;
        }
        else{
            Node *act = m_pRoot;
            while (act->getNext() != m_tail)
                act = act->getNext();
            delete m_tail;
            m_tail = act;
            m_tail->setNext(m_pRoot);
        }
        m_size--;
        return result;
    }

    //operator[]
    value_type &operator[](size_t index) override{
        shared_lock<shared_mutex> lock(m_mtx);
        if (index >= m_size) throw out_of_range("Indice fuera de rango");
        Node *act = m_pRoot;
        for (size_t i = 0; i < index; ++i)
            act = act->getNext();
        return act->getDataRef();
    }

    //size
    size_t size() const override{
        shared_lock<shared_mutex> lock(m_mtx);
        return m_size;
    }

    //iteradores end y begin
    forward_iterator begin() { return forward_iterator(this, m_pRoot); }
    forward_iterator end()   { return forward_iterator(this, nullptr, true); }

    //foreach
    template <typename Func, typename... Args>
    void ForEach(Func func, Args &&...args){
        unique_lock<shared_mutex> lock(m_mtx);
        if (m_size == 0) return;
        for (auto &item : *this)
            func(item, std::forward<Args>(args)...);
    }

    //circularForEach
    template <typename Func, typename... Args>
    void circularForEach(size_t vueltas, Func func, Args &&...args){
        unique_lock<shared_mutex> lock(m_mtx);
        if (!m_pRoot || vueltas == 0) return;
        Node  *act   = m_pRoot;
        size_t pasos = m_size * vueltas;
        for (size_t i = 0; i < pasos; ++i){
            func(act->getDataRef(), std::forward<Args>(args)...);
            act = act->getNext();
        }
    }

    //operator<<, mostrar y tambien el root tambien 
    friend ostream &operator<<(ostream &os, const CircularLinkedList &list){
        shared_lock<shared_mutex> lock(list.m_mtx);
        os << "[";
        if (list.m_pRoot){
            Node *act = list.m_pRoot;
            do {
                os << "(" << act->getData() << "," << act->getRef() << ")";
                act = act->getNext();
                if (act != list.m_pRoot) os << ",";
            } while (act != list.m_pRoot);
        }
        os << "]";
        if (list.m_pRoot)
            os << " ->root(" << list.m_pRoot->getData() << ")";
        return os;
    }

    //operator>> ,leer e insertar
    friend istream &operator>>(istream &is, CircularLinkedList &list){
        char ch;
        if (!(is >> ch) || ch != '['){
            is.clear(ios_base::failbit);
            return is;
        }
        value_type val;
        Ref        ref;
        char       comma, parenClose;
        while (is >> ch && ch != ']')
            if (ch == '(')
                if (is >> val >> comma >> ref >> parenClose)
                    if (comma == ',' && parenClose == ')')
                        list.insert(val, ref);
        is.ignore(numeric_limits<streamsize>::max(), '\n');
        return is;
    }
};

#endif