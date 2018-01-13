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
    PoolNode* node = (PoolNode*)malloc(sizeof(PoolNode));
    node->pool = t_pool;
    avl_insert(tree, &node->avl, cmp_func);
}

inline void remove(struct avl_tree* tree, void* t_pool) {
    PoolNode query;
    query.pool = t_pool;
    avl_node* res = avl_search(tree, &query.avl, cmp_func);
    PoolNode* poolNode = _get_entry(res, PoolNode, avl);
    avl_remove(tree, res);
    free(poolNode);
}

inline void* first(struct avl_tree* tree) {
    PoolNode* firstPool = _get_entry(avl_first(tree), PoolNode, avl);
    return firstPool ? firstPool->pool : nullptr;
}

}
