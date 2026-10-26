#ifndef STACK_H
#define STACK_H

#include <stdexcept>

// ------------------------------------------------------------
// Stack<T>  -  singly linked list based stack (LIFO)
// Used by UndoManager to keep the editing history.
// All operations are O(1) except clear() which is O(n).
// ------------------------------------------------------------

template <typename T>
class Stack {
private:
    struct Node {
        T data;
        Node* next;
        Node(const T& value, Node* nxt) : data(value), next(nxt) {}
    };

    Node* topNode;
    int count;

public:
    Stack() : topNode(nullptr), count(0) {}

    ~Stack() { clear(); }

    // Copying a stack of raw pointers safely would need a deep copy;
    // the editor never needs it, so it is disabled.
    Stack(const Stack&) = delete;
    Stack& operator=(const Stack&) = delete;

    void push(const T& value) {
        topNode = new Node(value, topNode);
        ++count;
    }

    void pop() {
        if (isEmpty()) {
            throw std::underflow_error("Stack::pop() called on an empty stack");
        }
        Node* old = topNode;
        topNode = topNode->next;
        delete old;
        --count;
    }

    T& top() {
        if (isEmpty()) {
            throw std::underflow_error("Stack::top() called on an empty stack");
        }
        return topNode->data;
    }

    const T& top() const {
        if (isEmpty()) {
            throw std::underflow_error("Stack::top() called on an empty stack");
        }
        return topNode->data;
    }

    bool isEmpty() const { return topNode == nullptr; }

    int size() const { return count; }

    void clear() {
        while (topNode != nullptr) {
            Node* old = topNode;
            topNode = topNode->next;
            delete old;
        }
        count = 0;
    }
};

#endif 