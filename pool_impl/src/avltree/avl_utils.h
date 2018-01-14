#ifndef __AVL_UTILS_H__
#define __AVL_UTILS_H__

extern "C" {
#include "avltree.h"
}
#include "cstdlib"

namespace avl {

struct PoolNode {
    struct avl_node avl;
    void *pool;
};

int cmp_func(struct avl_node *a, struct avl_node *b, void *aux) {
    struct PoolNode *aa, *bb;
    aa = _get_entry(a, struct PoolNode, avl);
    bb = _get_entry(b, struct PoolNode, avl);

    if (aa->pool < bb->pool)
        return -1;
    else if (aa->pool > bb->pool)
        return 1;
    else
        return 0;
}

inline void insert(struct avl_tree* tree, void* t_pool) {
    PoolNode* node = (PoolNode*) std::malloc(sizeof(PoolNode));
    node->pool = t_pool;
    avl_insert(tree, &node->avl, cmp_func);
}

inline void remove(struct avl_tree* tree, void* t_pool) {
    PoolNode query;
    query.pool = t_pool;
    avl_node* res = avl_search(tree, &query.avl, cmp_func);
    PoolNode* poolNode = _get_entry(res, PoolNode, avl);
    avl_remove(tree, res);
    std::free(poolNode);
}

inline void* first(struct avl_tree* tree) {
    PoolNode* firstPool = _get_entry(avl_first(tree), PoolNode, avl);
    return firstPool ? firstPool->pool : nullptr;
}

}

namespace avl2 {

struct PageNode {
    struct avl_node avl;
    void* pool;
    size_t num;
};

int cmp_func2(struct avl_node *a, struct avl_node *b, void *aux) {
    struct PageNode *aa, *bb;
    aa = _get_entry(a, struct PageNode, avl);
    bb = _get_entry(b, struct PageNode, avl);

    if (aa->pool < bb->pool)
        return -1;
    else if (aa->pool > bb->pool)
        return 1;
    else
        return 0;
}

inline void insert2(struct avl_tree* tree, void* t_page) {
    PageNode* node = (PageNode*) std::malloc(sizeof(PageNode));
    node->pool = t_page;
    node->num = 1;
    avl_insert(tree, &node->avl, cmp_func2);
}

inline avl_node* get2(struct avl_tree* tree , void* t_page) {
    PageNode query;
    query.pool = t_page;
    return avl_search(tree, &query.avl, cmp_func2);
}

inline void remove2(struct avl_tree* tree, avl_node* res) {
    PageNode* poolNode = _get_entry(res, PageNode, avl);
    avl_remove(tree, res);
    std::free(poolNode);
}

inline void* first2(struct avl_tree* tree) {
    PageNode* firstPool = _get_entry(avl_first(tree), PageNode, avl);
    return firstPool ? firstPool->pool : nullptr;
}

}

#endif // __AVL_UTILS_H__
