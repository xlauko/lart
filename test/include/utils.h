#include <stdio.h>
#include <stdlib.h>
#include <execinfo.h>

void reachable()
{
    fprintf( stderr, "lart-reachable\n" );
}
void unreachable()
{
    fprintf( stderr, "lart-unreachable\n" );
}

#define REACHABLE reachable();
#define UNREACHABLE unreachable();

#define noinline __attribute__((__noinline__))

void __lart_cancel(void);

void assume(int cond) {
  if (!cond)
    __lart_cancel();
}