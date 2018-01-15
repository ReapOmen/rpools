#ifndef __AVL_UTILS_H__
#define __AVL_UTILS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "avltree.h"
#include "cstdlib"

struct PoolNode {
    struct avl_node avl;
    void *pool;
};

int pool_cmp_func(struct avl_node *a, struct avl_node *b, void *aux) {
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

inline void pool_insert(struct avl_tree* tree, void* t_pool) {
    PoolNode* node = (PoolNode*) std::malloc(sizeof(PoolNode));
    node->pool = t_pool;
    avl_insert(tree, &node->avl, pool_cmp_func);
}

inline void pool_remove(struct avl_tree* tree, void* t_pool) {
    PoolNode query;
    query.pool = t_pool;
    avl_node* res = avl_search(tree, &query.avl, pool_cmp_func);
    PoolNode* poolNode = _get_entry(res, PoolNode, avl);
    avl_remove(tree, res);
    std::free(poolNode);
}

inline void* pool_first(struct avl_tree* tree) {
    PoolNode* firstPool = _get_entry(avl_first(tree), PoolNode, avl);
    return firstPool ? firstPool->pool : nullptr;
}

struct PageNode {
    struct avl_node avl;
    void* pool;
    size_t num;
};

int page_cmp_func(struct avl_node *a, struct avl_node *b, void *aux) {
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

inline void page_insert(struct avl_tree* tree, void* t_page) {
    PageNode* node = (PageNode*) std::malloc(sizeof(PageNode));
    node->pool = t_page;
    node->num = 1;
    avl_insert(tree, &node->avl, page_cmp_func);
}

inline avl_node* page_get(struct avl_tree* tree , void* t_page) {
    PageNode query;
    query.pool = t_page;
    return avl_search(tree, &query.avl, page_cmp_func);
}

inline void page_remove(struct avl_tree* tree, struct avl_node* res) {
    PageNode* poolNode = _get_entry(res, PageNode, avl);
    avl_remove(tree, res);
    std::free(poolNode);
}

#ifdef __cplusplus
}
#endif

#endif // __AVL_UTILS_H__
