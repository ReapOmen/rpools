#ifndef __NODE_H__
#define __NODE_H__

struct Node {
    Node* next;

    Node(Node* t_next = nullptr) : next(t_next) {}
};

#endif // __NODE_H__
