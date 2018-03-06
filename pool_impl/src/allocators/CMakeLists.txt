add_library(linkedpools SHARED
  ${SRC}/avltree/avltree.c
  ${SRC}/avltree/avl_utils.c
  ${SRC}/tools/LMLock.cpp
  ${SRC}/allocators/GlobalLinkedPool.cpp
  ${SRC}/allocators/NSGlobalLinkedPool.cpp)
install(TARGETS linkedpools DESTINATION lib)
