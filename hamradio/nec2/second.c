#include <sys/time.h>

void second_ (double *sec)
{
    struct timeval tv;

    gettimeofday (&tv, 0);
    *sec = (double) tv.tv_sec;
}
