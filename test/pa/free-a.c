// RUN: %testrun %lartcc pa -lz3 %s -o %t | %filecheck %s

#include <lamp-pointers.h>
#include <assert.h>
#include <stddef.h>
#include "utils.h"

int main()
{
   int* ptr = (int *)( __lamp_lift_objid( malloc( sizeof( int ) ) ) );
   free( ptr );
   REACHABLE // CHECK: lart-reachable
}
