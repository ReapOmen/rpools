#ifndef __AVL_UTILS_H__
#define __AVL_UTILS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "avltree.h"
#include <stdlib.h>

struct PoolNode {
    struct avl_node avl;
    void *pool;
};

int pool_cmp_func(struct avl_node *a, struct avl_node *b, void *aux);

size_t pool_count(const struct avl_tree* tree);

void pool_insert(struct avl_tree* tree, void* t_pool);

void pool_remove(struct avl_tree* tree, void* t_pool);

void* pool_first(struct avl_tree* tree);

struct PageNode {
    struct avl_node avl;
    void* pool;
    size_t num;
};

int page_cmp_func(struct avl_node *a, struct avl_node *b, void *aux);

void page_insert(struct avl_tree* tree, void* t_page);

struct avl_node* page_get(struct avl_tree* tree , void* t_page);

void page_remove(struct avl_tree* tree, struct avl_node* res);

#ifdef __cplusplus
}
#endif

#endif // __AVL_UTILS_H__
