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
