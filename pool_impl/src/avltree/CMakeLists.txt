# Prepare "avltree" library for linking
add_library(avltree
  ${SRC}/avltree/avltree.c
  ${SRC}/avltree/avl_utils.c)
set_target_properties(avltree PROPERTIES PUBLIC_HEADER ${SRC}/avltree/avl_utils.h)
