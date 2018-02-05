#include "avl_utils.h"

int pool_cmp_func(struct avl_node *a, struct avl_node *b, void *aux) {
    struct PoolNode *aa, *bb;
    aa = _get_entry(a, struct PoolNode, avl);
    bb = _get_entry(b, struct PoolNode, avl);

    if ((size_t) aa->pool < (size_t) bb->pool)
        return -1;
    else if ((size_t) aa->pool > (size_t) bb->pool)
        return 1;
    else
        return 0;
}

size_t pool_count(const struct avl_tree* tree) {
    struct avl_node *cursor;
    size_t count = 0;
    cursor = avl_first_const(tree);
    while (cursor) {
        ++count;
        cursor = avl_next(cursor);
    }
    return count;
}

void pool_insert(struct avl_tree* tree, void* t_pool) {
    struct PoolNode* node = (struct PoolNode*) malloc(sizeof(struct PoolNode));
    node->pool = t_pool;
    avl_insert(tree, &node->avl, pool_cmp_func);
}

void pool_remove(struct avl_tree* tree, void* t_pool) {
    struct PoolNode query;
    query.pool = t_pool;
    struct avl_node* res = avl_search(tree, &query.avl, pool_cmp_func);
    struct PoolNode* poolNode = _get_entry(res, struct PoolNode, avl);
    avl_remove(tree, res);
    free(poolNode);
}

void* pool_first(struct avl_tree* tree) {
    struct PoolNode* firstPool = _get_entry(tree->root, struct PoolNode, avl);
    return firstPool ? firstPool->pool : NULL;
}

int page_cmp_func(struct avl_node *a, struct avl_node *b, void *aux) {
    struct PageNode *aa, *bb;
    aa = _get_entry(a, struct PageNode, avl);
    bb = _get_entry(b, struct PageNode, avl);

    if ((size_t) aa->pool < (size_t) bb->pool)
        return -1;
    else if ((size_t) aa->pool > (size_t) bb->pool)
        return 1;
    else
        return 0;
}

void page_insert(struct avl_tree* tree, void* t_page) {
    struct PageNode* node = (struct PageNode*) malloc(sizeof(struct PageNode));
    node->pool = t_page;
    node->num = 1;
    avl_insert(tree, &node->avl, page_cmp_func);
}

struct avl_node* page_get(struct avl_tree* tree , void* t_page) {
    struct PageNode query;
    query.pool = t_page;
    return avl_search(tree, &query.avl, page_cmp_func);
}

void page_remove(struct avl_tree* tree, struct avl_node* res) {
    struct PageNode* poolNode = _get_entry(res, struct PageNode, avl);
    avl_remove(tree, res);
    free(poolNode);
}
