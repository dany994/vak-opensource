#include <sys/time.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#define N	10

int t0, t1, counter;

int getusec ()
{
	struct timeval tv;

	gettimeofday (&tv, 0);
	return tv.tv_sec * 1000000 + tv.tv_usec;
}

/*
 * Функция вызывается каждые 4 миллисекунды реального времени.
 */
static void cpu_sigalarm (int signum)
{
	if (counter == 0) {
		/* Первый отсчёт времени. */
		t0 = getusec ();
	}
	++counter;
	if (counter <= N)
		return;

	/* Последний отсчёт после N интервалов. */
	t1 = getusec ();
	printf ("Measured interval is %.2f msec.\n",
		(t1 - t0) / 1000.0 / N);
	exit (0);
}

int main (int argc, char **argv)
{
	struct itimerval itv;
	int msec;

	if (argc != 2) {
		fprintf (stderr, "Usage:\n");
		fprintf (stderr, "        timerbench msec\n");
		exit (1);
	}
	msec = strtol (argv[1], 0, 0);

	counter = 0;
	signal (SIGALRM, cpu_sigalarm);

	itv.it_interval.tv_sec = 0;
	itv.it_interval.tv_usec = msec * 1000;;
	itv.it_value.tv_sec = 0;
	itv.it_value.tv_usec = msec * 1000;
	printf ("Set interval timer for %d msec.\n", msec);
	if (setitimer (ITIMER_REAL, &itv, 0) < 0) {
		perror ("setitimer");
		exit (1);
	}
	for (;;)
		pause ();
}
