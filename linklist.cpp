#include"./linklist.h"

    template<typename T>
    void ListNode<T>::insert_at_next(ListNode<T>* node)
    {
        node->prev = this;
        node->next = this->next;
        
        this->next->prev = node;
        this->next = node;
    }

