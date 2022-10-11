#include <svcomp.h>

extern void abort(void);
extern void __assert_fail(const char *, const char *, unsigned int, const char *) __attribute__ ((__nothrow__ , __leaf__)) __attribute__ ((__noreturn__));
void reach_error() { __assert_fail("0", "array_tiling_poly6.c", 3, "reach_error"); }
extern void abort(void);
void assume_abort_if_not(int cond) {
  if(!cond) {abort();}
}
void __VERIFIER_assert(int cond) { if(!(cond)) { ERROR: {reach_error();abort();} } }
int main()
{
  int S=__VERIFIER_nondet_int();
  assume_abort_if_not(S>1);
  long long i;
  long long a[S];

  for(i=0;i<S;i++)
    a[i]=((i-1)*(i+1));

  for(i=0;i<S;i++)
    a[i]=a[i]-(i*i);

  for(i=0;i<S;i++)
    __VERIFIER_assert(a[i]==-1);
  return 0;
}

