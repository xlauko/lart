// RUN: %testrun %lartcc++ pa -lz3 %s -o %t | %filecheck %s

#include <lamp-pointers.h>
#include <assert.h>
#include "utils.h"

struct node
{
    int val;
    node * next = nullptr;
};

int main()
{
    node a{ .val = 1 }, b{ .val = 2 };
    auto ap = (node *)__lamp_lift_objid( &a );
    auto bp = (node *)__lamp_lift_objid( &b );
    ap->next = bp;
    assert( ap->val == 1 );
    assert( ap->next == bp );
    assert( ap->next->val == 2 );
    assert( ap->next->next == nullptr );
    REACHABLE // CHECK: lart-reachable
}