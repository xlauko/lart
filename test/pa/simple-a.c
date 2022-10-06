// RUN: %testrun %lartcc pa -lz3 %s -o %t | %filecheck %s
// REQUIRES: pa

#include <lamp-pointers.h>
#include <assert.h>
#include "utils.h"

int main()
{
  int x[3] = { 1, 2, 3 };
  int *p = (int*)__lamp_lift_objid( &x );
  int *o = p + 1;
  assert( *o == 2 );
  REACHABLE // CHECK: lart-reachable
}
