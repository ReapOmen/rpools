#include "rpools/custom_new/custom_new_delete.hpp"

int main() {
    // this is used exactly like operator new, except you can
    // provide an additional parameter for the alignment
    auto x = static_cast<int*>(custom_new(sizeof(int), alignof(int)));
    custom_delete(x);
    // same as std::no_throw operator new
    auto y = static_cast<int*>(custom_new_no_throw(sizeof(int)));
    custom_delete(y);
}
