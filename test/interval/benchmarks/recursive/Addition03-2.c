#include <svcomp.h>

extern void abort(void);
extern void __assert_fail(const char *, const char *, unsigned int, const char *) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__noreturn__));
void reach_error() { __assert_fail("0", "Addition03-2.c", 3, "reach_error"); }

/*
 * Recursive implementation integer addition.
 *
 * Author: Matthias Heizmann
 * Date: 2013-07-13
 *
 */

int addition(int m, int n) {
    if (n == 0) {
        return m;
    }
    if (n > 0) {
        return addition(m+1, n-1);
    }
    if (n < 0) {
        return addition(m-1, n+1);
    }
}


int main() {
    int m = __VERIFIER_nondet_int();
    int n = __VERIFIER_nondet_int();
    int result = addition(m,n);
    if (m < 100 || n < 100 || result >= 200) {
        return 0;
    } else {
        ERROR: {reach_error();abort();}
    }
}
