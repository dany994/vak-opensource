#include <stdio.h>
#include <stdlib.h>

extern void *G[];

#define STKSZ   (16*1024)       // Number of words reserved for BCPL stack.
#define FTSZ    20              // Max number of opened files.

/* Assign x = AX */
#define GET_A(x)   asm volatile("" : "=a" (x))
#define SET_A(x)   asm volatile("" : : "a" (x))

static void *stack [STKSZ];     // BCPL stack area.
static FILE *ft [FTSZ];
static int fi, fo;

void finish()
{
    exit(0);
}

int main(argc, argv, envv)
    int argc;
    char **argv;
    char **envv;
{
    stack[0] = stack;
    stack[1] = finish;
    stack[2] = (void*) argc;
    stack[3] = argv;
    stack[4] = envv;

    ft[0] = stdin;
    ft[1] = stdout;
    ft[2] = stderr;
    fi = 0;
    fo = 1;

    asm volatile (
    "   mov     %0, %%ebp \n"
    "   mov     %1, %%edi \n"
    "	mov	4(%%edi), %%eax \n"
    "	jmp	*%%eax \n"
    : : "r" (stack), "r" (G) : "ax");

    return 0;
}

void stop()
{
    int status;

    GET_A(status);
    exit(status);
}

#if 0
extern int *M;

int
getbyte(s, i)
    int s, i;
{
    int w = M[s + i / 4];
    int m = (i % 4) ^ 3;
    w = w >> (8 * m);
    return w & 255;
}

void
putbyte(s, i, ch)
{
    int p = s + i / 4;
    int m = (i % 4) ^ 3;
    int w = M[p];
    int x = 0xff;
    x = x << (8 * m);
    x = x ^ 0xffffffff;
    w = w & x;
    x = ch;
    x = x & 0xff;
    x = x << (8 * m);
    w = w | x;
    M[p] = w;
}

static char *
cstr(s)
{
    char *st;
    int n, i;

    n = getbyte(s, 0);
    st = malloc(n + 1);
    for (i = 1; i <= n; i++)
        st[i - 1] = getbyte(s, i);
    st[n] = 0;
    return st;
}

static int
ftslot()
{
    int i;

    for (i = 3; i < FTSZ; i++)
        if (ft[i] == NULL)
            return i;
    return -1;
}
#endif

void
findinput()
{
    int s;

    GET_A(s);
#if 1
    printf("--- %s() called\n", __func__);
    SET_A(0);
#else
    char *st = cstr(s);
    int x;

    x = ftslot();
    if (x != -1) {
        ft[x] = fopen(st, "r");
        if (ft[x] == NULL)
            x = -1;
    }
    free(st);
    SET_A(x + 1);
#endif
}

void
findoutput()
{
    int s;

    GET_A(s);
#if 1
    printf("--- %s() called\n", __func__);
    SET_A(0);
#else
    char *st = cstr(s);
    int x;

    x = ftslot();
    if (x != -1) {
        ft[x] = fopen(st, "w");
        if (ft[x] == NULL)
            x = -1;
    }
    free(st);
    SET_A(x + 1);
#endif
}

void
selectinput()
{
    int x;

    GET_A(x);
    fi = x - 1;
}

void
selectoutput()
{
    int x;

    GET_A(x);
    fo = x - 1;
}

void
input()
{
    SET_A(fi + 1);
}

void
output()
{
    SET_A(fo + 1);
}

void
rdch()
{
    SET_A(fgetc(ft[fi]));
}

void
wrch()
{
    int c;

    GET_A(c);
    fputc(c, ft[fo]);
}

void
unrdch()
{
    int c;

    GET_A(c);
    ungetc(c, ft[fo]);
}

void
endread()
{
    if (fi > 2) {
        fclose(ft[fi]);
        ft[fi] = NULL;
    }
    fi = 0;
}

void
endwrite()
{
    if (fo > 2) {
        fclose(ft[fo]);
        ft[fo] = NULL;
    } else
        fflush(ft[fo]);
    fo = 1;
}
