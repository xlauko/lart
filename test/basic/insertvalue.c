/* TAGS: error todo sym c++ */
/* VERIFY_OPTS: --symbolic */
/* CC_OPTS: -O1 */

#include <assert.h>

extern "C" {
int __lamp_any_i32();
}

struct S { int a, b, c; };

S foo() {
    S s;
    s.b = __lamp_any_i32();
    return s;
}

int main() {
    assert( foo().b == 0 ); /* ERROR */
}
