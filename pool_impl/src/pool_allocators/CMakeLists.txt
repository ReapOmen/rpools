add_library(linkedpools SHARED
  ${SRC}/avltree/avltree.c
  ${SRC}/avltree/avl_utils.c
  ${SRC}/tools/LMLock.cpp
  ${SRC}/pool_allocators/GlobalLinkedPool.cpp
  ${SRC}/pool_allocators/NSGlobalLinkedPool.cpp)
install(TARGETS linkedpools DESTINATION /usr/local/lib)
