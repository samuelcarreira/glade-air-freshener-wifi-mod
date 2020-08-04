
/*
 * Simple kind of stack 
 * After the stack is full, delete the first item to prevent stack overflow
 * New items (push operation) are always stored on top of the stack
 * 
 * based on Efstathios Chatzikyriakidis work. Library: StackArray.h
 */

#ifndef STACK_H
#define STACK_H

#include <stdint.h>  // because uint16_t type declarations on platform.io

template <typename T>
class Stack {
   public:
    Stack(uint8_t maxMapacity);

    ~Stack();

    uint8_t capacity;

    uint8_t size = 0;

    void push(const T item);

    T *array;

   private:
    uint8_t top = 0;
};

template <typename T>
Stack<T>::Stack(uint8_t maxCapacity) {
    if (maxCapacity > 255 || maxCapacity < 1) {
        maxCapacity = 1;
    }

    capacity = maxCapacity;
    array = (T *)malloc(sizeof(T) * capacity);  // allocate all memory at init

    if (array == NULL) {
        // STACK: insufficient memory to initialize stack.
        capacity = 1;
        array = (T *)malloc(sizeof(T) * capacity);
    }
}

template <typename T>
Stack<T>::~Stack() {
    free(array);  // deallocate the array of the stack.

    array = NULL;  // set stack's array pointer to nowhere.
    size = 0;
    top = 0;
}

template <typename T>
void Stack<T>::push(const T item) {
    if (top > (capacity - 1)) {
        for (int i = 0; i < capacity - 1; i++) {
            array[i] = array[i + 1];
        }

        top = (capacity - 1);
    }

    array[top] = item;
    top++;

    if (size < capacity) {
        size++;
    }
}

#endif  // STACK_H