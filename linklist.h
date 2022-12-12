#ifndef LINKLIST
#define LINKLIST
#include <cstdlib>
#include <cassert>

template<typename T>
class ListNode
{
    public:
    ListNode<T>* prev;
    ListNode<T>* next;
    T* data;
    ListNode(T* d, ListNode<T>* prev=nullptr, ListNode<T>* next=nullptr):data(d), prev(prev), next(next)
    {}
    ListNode(): data(nullptr), prev(nullptr), next(nullptr)
    {}
    ListNode(ListNode<T>* prev, ListNode<T>* next): data(nullptr), prev(prev), next(next)
    {}
    void insert_at_next(ListNode<T>* node)
    {
        node->prev = this;
        node->next = this->next;
        
        this->next->prev = node;
        this->next = node;
    }
    
    void insert_at_prev(ListNode<T>* node)
    {
        node->next = this;
        node->prev = this->prev;

        this->prev->next = node;
        this->prev = node;
    }

    T* remove_self()
    {
        this->prev->next = this->next;
        this->next->prev = this->prev;
        this->prev = nullptr;
        this->next = nullptr;
        return data;
    }

};

template<typename subclass>
class LinkList
{
    private:
            ListNode<subclass> head;
            ListNode<subclass> tail;
            u_int64_t size;
    public:
            LinkList(): size(0)
            {
                head.next = &tail;
                head.prev = nullptr;
                tail.prev = &head;
                tail.next = nullptr;
            }

            ListNode<subclass>* get(u_int64_t i)
            {
                assert(i < size);
                ListNode<subclass>* go = head.next;
                while(i > 0)
                {
                    go = go->next;
                    i -= 1;
                }
                return go;
            }
            
            u_int64_t total_size()
            {
                return size;
            }

            void insert_at_begin(subclass* data)
            {
                    ListNode<subclass>* node = new ListNode<subclass>(data);
                    size += 1;
                    head.insert_at_next(node);
            }
             
            void insert_at_tail(subclass* data)
            {
                    ListNode<subclass>* node = new ListNode<subclass>(data);
                    size += 1;
                    tail.insert_at_prev(node);
            }

            ListNode<subclass>* begin()
            {
                return head.next;
            }

            ListNode<subclass>* end()
            {
                return &tail;
            }

            ListNode<subclass>* remove(ListNode<subclass>* t)
            {
                ListNode<subclass>* next = t->next;
                size -= 1;
                subclass* data = t->remove_self();
                delete data;
                return next;
            }
};
#endif
