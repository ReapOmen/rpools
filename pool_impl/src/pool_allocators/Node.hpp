#ifndef __NODE_H__
#define __NODE_H__

/**
 *  Represents a singly linked list node.
 */
struct Node {
    /** Next element in the list. */
    Node* next;
    /** Create a `Node` which points to another `Node`
     *  @param t_next the next `Node` (default: `nullptr`)
     */
    Node(Node* t_next = nullptr) : next(t_next) {}
};

#endif // __NODE_H__
