<<<<<<< HEAD
#include <svcomp.h>

=======
>>>>>>> 36dbba2 (test: Svcomp benchmarks for interval domain evaluation.)
extern void abort(void);
extern void __assert_fail(const char *, const char *, unsigned int, const char *) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__noreturn__));
void reach_error() { __assert_fail("0", "Fibonacci01-1.c", 3, "reach_error"); }

/*
 * Recursive computation of fibonacci numbers.
<<<<<<< HEAD
 *
 * Author: Matthias Heizmann
 * Date: 2013-07-13
 *
 */

=======
 * 
 * Author: Matthias Heizmann
 * Date: 2013-07-13
 * 
 */

extern int __VERIFIER_nondet_int(void);

>>>>>>> 36dbba2 (test: Svcomp benchmarks for interval domain evaluation.)

int fibonacci(int n) {
    if (n < 1) {
        return 0;
    } else if (n == 1) {
        return 1;
    } else {
        return fibonacci(n-1) + fibonacci(n-2);
    }
}


int main() {
    int x = __VERIFIER_nondet_int();
    if (x > 46 || x == -2147483648) {
        return 0;
    }
    int result = fibonacci(x);
    if (result >= x - 1) {
        return 0;
    } else {
        ERROR: {reach_error();abort();}
    }
}
<<<<<<< HEAD

<<<<<<< HEAD
=======

>>>>>>> 794a251 (test: Svcomp benchmarks for interval domain evaluation.)
=======
    

>>>>>>> 36dbba2 (test: Svcomp benchmarks for interval domain evaluation.)
