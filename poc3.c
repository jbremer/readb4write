#include <stdio.h>
#include <stdlib.h>

int main()
{
    int *a = (int *) malloc(sizeof(int) * 3);
    a[1] = 0;
    printf("a0: %d\n", *a);
    printf("a1: %d\n", a[1]);
    printf("a2: %d\n", a[2]);
    return 0;
}
