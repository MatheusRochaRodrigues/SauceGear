#pragma once
#include <iostream>
#include <atomic>

//Pool de objetos
template<typename T>
class ObjectPool2
{
    struct Node     // POD
    {
        T object;
        Node* next;
    };

    std::atomic<Node*> freeList{ nullptr };

public:

    T* Acquire()
    {
        Node* n = freeList.load(std::memory_order_acquire);

        while (n)
        {
            if (freeList.compare_exchange_weak(n, n->next))     // CAS (Compare-And-Swap)   Thread
                return &n->object;
        }

        return &((new Node())->object);                                  //new (&n->object) T();  // constrói Task corretamente
    }

    void Release(T* obj)
    {
        //1
        //Node* n = reinterpret_cast<Node*>(obj);     // PERIGOSO - ANALISE COM CUIDADO

        //2
        Node* n = ObjectToNode(obj);
        // destrói o objeto antes de reutilizar
        obj->~T();


        Node* head = freeList.load(std::memory_order_relaxed);

        do {
            n->next = head;
        } while (!freeList.compare_exchange_weak(head, n));
    }



    void Preallocate(size_t count)
    {
        for (size_t i = 0; i < count; i++)
        {
            Node* n = new Node();

            Node* head = freeList.load(std::memory_order_relaxed);
            do {
                n->next = head;
            } while (!freeList.compare_exchange_weak(head, n));
        }
    }

    // more Performance
    void PreallocateBlock(size_t count)
    {
        Node* block = (Node*)::operator new(sizeof(Node) * count);      //Só aloca memória bruta, e NÃO chama construtor    -> raw memory

        for (size_t i = 0; i < count; i++)
        {
            Node* n = &block[i];

            new (n) Node(); // ESSENCIAL

            Node* head = freeList.load(std::memory_order_relaxed);
            do {
                n->next = head;
            } while (!freeList.compare_exchange_weak(head, n));
        }
    }

    void PreallocateBlockWithContructor(size_t count)
    {
        Node* block = (Node*)::operator new(sizeof(Node) * count);

        for (size_t i = 0; i < count; i++)
        {
            Node* n = &block[i];

            new (n) Node(); //   constrói o objeto

            Node* head = freeList.load(std::memory_order_relaxed);
            do {
                n->next = head;
            } while (!freeList.compare_exchange_weak(head, n));
        }
    }

    Node* ObjectToNode(T* obj)
    {
        return reinterpret_cast<Node*>(
            reinterpret_cast<char*>(obj) - offsetof(Node, object)
            );
    }
};