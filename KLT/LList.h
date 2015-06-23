/*

 Copyright (C) 2004, Aseem Agarwala, roto@agarwala.org

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or (at
 your option) any later version.

 This program is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 USA

 */

#ifndef LLIST_H
#define LLIST_H

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

template<class T> class LList
{

    // note: class T needs to have a next field,and be accessible to me

public:

    LList()
    {
        head = tail = current = NULL;
        count = 0;
    }

    ~LList();

    void AddToHead(T *node);
    void AddToTail(T *node);

    void SetIterationHead() const
    {
        current = head;
    }

    void SetIterationTail() const
    {
        current = tail;
    }

    inline const T *IterateNext() const;
    inline const T *IteratePrev() const;
    inline T *IterateNext();
    inline T *IteratePrev();
    inline T *IterateForward(const int i) const;

    inline T *popHead();
    inline T *popTail();

    void RemoveNode(T *node, int del = 1);
    // deletes removed node if del==1

    inline LList *CopyTo(LList<T>* in) const;

    int getSize() const
    {
        return count;
    }

    void clear();

    T *HeadPtr()
    {
        return head;
    }

    T *TailPtr()
    {
        return tail;
    }

    const T *HeadPtr() const
    {
        return head;
    }

    const T *TailPtr() const
    {
        return tail;
    }

    void InsertAfter(T* newNode, T* place);
    // Inserts newNode after place
    // Won't change iteration place

    void forgetContents();

    void copyPointers(const LList<T>* other)
    {
        head = other->head;
        tail = other->tail;
        current = other->current;
        count = other->count;
    }

    void append(const LList<T>* other);

    bool confirmSize();
    bool isInList(const T* n) const;

    void ReplaceNNodes(int n, T* here, const T* newNodes);
    // note: we copy the newNodes, don't just add them

    void MoveToTail(T* node);
    void MoveToHead(T* node);
    void MoveOneUp(T* node);
    void MoveOneDown(T* node);

private:

    T *head;
    T *tail;

protected:
    int count;
public:
    mutable T *current;
};

template<class T> inline const T *LList<T>::IterateNext() const
{

    if (current != NULL)
    {
        T *toReturn = current;
        current = current->next;
        return toReturn;
    }
    else
    {
        return NULL;
    }
}

template<class T> inline T *LList<T>::IterateForward(const int i) const
{
    for (int j = 0; j < i; j++)
        if (current == NULL)
            return NULL;
        else
            current = current->next;
    return current;
}

template<class T> inline T *LList<T>::IterateNext()
{

    if (current != NULL)
    {
        T *toReturn = current;
        current = current->next;
        return toReturn;
    }
    else
    {
        return NULL;
    }
}

template<class T> void LList<T>::ReplaceNNodes(int n, T* here,
        const T* newNodes)
{
    // nodes are replaced AFTER here
    assert(here);
    assert(newNodes);
    assert(newNodes + n);
    assert(isInList(here));
    int i;

    T* curr = here;
    T* tmp;
    for (i = 0; i < n; i++)
    {
        assert(curr);
        T* newN = new T;
        *newN = newNodes[i];
        tmp = curr->next;
        assert(tmp);
        curr->next = newN;
        newN->next = tmp->next;
        newN->prev = curr;
        if (tmp->next != NULL)
            tmp->next->prev = newN;
        curr = curr->next;
        delete tmp;
        tmp = NULL;
    }

}

template<class T> bool LList<T>::isInList(const T* n) const
{
    T* save = current;
    SetIterationHead();
    const T* curr;
    while ((curr = IterateNext()) != NULL)
        if (curr == n)
        {
            current = save;
            return 1;
        }

    // failed
    current = save;
    return 0;
}

template<class T> void LList<T>::AddToHead(T *node)
{
    assert(node != NULL);
    if (head != NULL)
    {

        T *tmp = head;

        head = node;
        head->next = tmp;
        head->prev = NULL;
        tmp->prev = head;

    }
    else
    {

        head = tail = node;
        node->prev = NULL;
        node->next = NULL;

    }
    count++;
}

template<class T> void LList<T>::AddToTail(T *node)
{
    assert(node != NULL);
    if (tail != NULL)
    {

        T *tmp = tail;

        tail = node;
        tail->prev = tmp;
        tail->next = NULL;
        tmp->next = tail;

    }
    else
    {

        head = tail = node;
        node->prev = NULL;
        node->next = NULL;

    }
    count++;
}

template<class T> void LList<T>::RemoveNode(T *node, int del)
{

    assert(node != NULL);
    assert(count > 0);

    count--;
    if ((node->prev == NULL) && (node->next == NULL))
    {
        // head and tail
        head = tail = NULL;
        if (del)
            delete node;
        current = NULL;
        return;
    }

    if (node->prev == NULL)
    {
        // head
        node->next->prev = NULL;
        head = node->next;
        if (current == node)
            current = head;
        if (del)
            delete node;
        return;
    }

    if (node->next == NULL)
    {
        // tail
        node->prev->next = NULL;
        tail = node->prev;
        if (current == node)
            current = NULL;
        if (del)
            delete node;
        return;
    }

    // middle
    if (current == node)
        current = node->next;
    node->prev->next = node->next;
    node->next->prev = node->prev;
    if (del)
        delete node;
    return;
}

template<class T> void LList<T>::clear()
{
    T *node;

    SetIterationHead();
    while ((node = IterateNext()) != NULL)
        delete node;
    head = tail = current = NULL;
    count = 0;
}

template<class T> void LList<T>::forgetContents()
{
    head = tail = current = NULL;
    count = 0;
}

template<class T> LList<T>::~LList()
{

    T *node;

    SetIterationHead();
    while ((node = IterateNext()) != NULL)
    {
        assert(node);
        delete node;
    }

}

template<class T> void LList<T>::InsertAfter(T* newNode, T* place)
{
    assert(newNode && place);
    newNode->next = place->next;
    newNode->prev = place;
    if (place->next != NULL)
        place->next->prev = newNode;
    place->next = newNode;
    ++count;
}

template<class T> void LList<T>::MoveToTail(T* node)
{
    assert(node);
    RemoveNode(node, 0);
    AddToTail(node);
}

template<class T> void LList<T>::MoveToHead(T* node)
{
    assert(node);
    RemoveNode(node, 0);
    AddToHead(node);
}

template<class T> void LList<T>::MoveOneUp(T* node)
{
    assert(node);
    T* prevNode = node->prev;
    if (!prevNode)
        return;
    RemoveNode(prevNode, 0);
    InsertAfter(prevNode, node);
}

template<class T> void LList<T>::MoveOneDown(T* node)
{
    assert(node);
    T* nextNode = node->next;
    if (!nextNode)
        return;
    RemoveNode(node, 0);
    InsertAfter(node, nextNode);
}

template<class T> inline const T *LList<T>::IteratePrev() const
{

    if (current != NULL)
    {
        T *toReturn = current;
        current = current->prev;
        return toReturn;
    }
    else
    {
        return NULL;
    }
}

template<class T> inline T *LList<T>::IteratePrev()
{

    if (current != NULL)
    {
        T *toReturn = current;
        current = current->prev;
        return toReturn;
    }
    else
    {
        return NULL;
    }
}

template<class T> inline LList<T> *LList<T>::CopyTo(LList<T>* in) const
{

    const T *node;

    SetIterationHead();
    while ((node = IterateNext()) != NULL)
    {
        in->AddToTail(new T(*node));
    }

    return in;
}

template<class T> inline T *LList<T>::popHead()
{
    if (head == NULL)
        return NULL;
    if (head == tail)
    {
        T* ret = head;
        head = tail = current = NULL;
        count--;
        ret->next = ret->prev = NULL;
        return ret;
    }

    T* ret = head;
    head = head->next;
    head->prev = NULL;
    count--;
    ret->next = ret->prev = NULL;
    return ret;
}

template<class T> inline T *LList<T>::popTail()
{
    if (tail == NULL)
        return NULL;
    if (head == tail)
    {
        T* ret = tail;
        head = tail = current = NULL;
        count--;
        ret->next = ret->prev = NULL;
        return ret;
    }

    T* ret = tail;
    tail = tail->prev;
    tail->next = NULL;
    count--;
    ret->next = ret->prev = NULL;
    return ret;
}

template<class T> inline void LList<T>::append(const LList<T>* other)
{
    if (tail == NULL)
    {
        assert(head==NULL);
        copyPointers(other);
        return;
    }

    assert(tail->next == NULL);
    tail->next = other->head;
    assert(other->head->prev == NULL);
    other->head->prev = tail;
    tail = other->tail;
    count += other->count;
}

template<class T> inline bool LList<T>::confirmSize()
{
    T* save = current, *curr;
    SetIterationHead();
    int c = 0;
    while ((curr = IterateNext()) != NULL)
        c++;
    assert(count == c);
    current = save;
    return 1;
}

#endif
