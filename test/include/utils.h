#include <stdio.h>

void reachable() { fprintf( stderr, "lart-reachable\n" ); }
void unreachable() { fprintf( stderr, "lart-unreachable\n" ); }

#define REACHABLE reachable();
#define UNREACHABLE unreachable();
