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

#define ASSERT( expr )                                                        \
    do {                                                                      \
            if ( !( expr ) ) {                                                \
                    fprintf( stderr,                                          \
                             "file %s: line %d (%s): assertion %s failed.\n", \
                             __FILE__,                                        \
                             __LINE__,                                        \
                             __PRETTY_FUNCTION__,                             \
                             #expr );                                         \
                    exit( EXIT_SUCCESS );                                     \
            };                                                                \
    } while ( 0 )

#define noinline __attribute__((__noinline__))
