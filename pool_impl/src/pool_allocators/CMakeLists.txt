add_library(linkedpool SHARED
  ${SRC}/avltree/avltree.c
  ${SRC}/avltree/avl_utils.c
  ${SRC}/tools/LMLock.cpp)
set_target_properties(linkedpool PROPERTIES PUBLIC_HEADER
  ${SRC}/pool_allocators/LinkedPool3.h)
