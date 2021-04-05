// RUN: %testrun %lartcc pa -lz3 %s -o %t | %filecheck %s

#include <lamp-pointers.h>

int main()
{
    int arr[3];
    int *p = (int*)__lamp_lift_objid( arr );
    int *b = p + 1;
}