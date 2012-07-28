// http://reversingonwindows.blogspot.com/2012/07/detecting-read-access-to-uninitialized.html

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX 16

typedef struct _CONTEXT {
    int arr[MAX];
    int a;
    int b;
    int c;
} CONTEXT;

void init(CONTEXT* ctx)
{
    memset(ctx->arr, 0, sizeof(ctx->arr[0]) * (MAX-1));
    ctx->a = 1;
}

void process(CONTEXT* ctx)
{
    int trash;

    for (int i = 0; i < MAX; i++)
    {
        trash = ctx->arr[i];
    }
}

void process2(CONTEXT* ctx)
{
    ctx->b = ctx->c;
}

void process3(int num)
{
    int trash;

    if (num)
        trash = num;
}

int main(int argc, char *argv[])
{
    CONTEXT ctx;

    // Erroneously initializes context. The last element of arr member remains unitialized.
    // b and c members remain uninitialized, too.
    init(&ctx);

    // Accesses to each element of the array. Read-before-write error should be reported in this function.
    process(&ctx);

    // Copies c to b but c is uninitialized. Read-before-write error should be reported in this function.
    process2(&ctx);

    // This contains no read-before-write bug.
    process3(ctx.a);
}

