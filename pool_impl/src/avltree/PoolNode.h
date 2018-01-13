#include "avltree.h"

#ifdef __cplusplus
extern "C" {
#endif

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

#ifdef __cplusplus
}
#endif
