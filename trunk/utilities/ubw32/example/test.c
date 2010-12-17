/*
	Example "blinkenlights" program for the UBW32 and C32 compiler.
	Demonstrates software PWM, use of floating-point library, etc.

	IMPORTANT: file 'procdefs.ld' absolutely must Must MUST be present
	in the working directory when compiling code for the UBW32!  Failure
	to do so will almost certainly result in your UBW32 getting 'bricked'
	and requiring re-flashing the bootloader (which, if you don't have a
	PICkit 2 or similar PIC programmer, you're screwed).
	YOU HAVE BEEN WARNED.

	2/19/2009 - Phillip Burgess - pburgess@dslextreme.com
*/

/*#include <p32xxxx.h>*/
/*#include <plib.h>*/
/*#include <math.h>*/

#define BIT_0	(1 << 0)
#define BIT_1	(1 << 1)
#define BIT_2	(1 << 2)
#define BIT_3	(1 << 3)

static void pwm (int a, int b, int c, int d)
{
	int bits, i;

	bits = a ? BIT_0 : 0;
	if (b > 0) bits |= BIT_1;
	if (c > 0) bits |= BIT_2;
	if (d > 0) bits |= BIT_3;
	mPORTEClearBits (bits);

	for (i=0; i<=1000; i++) {
		bits = (i >= a) ? BIT_0 : 0;
		if (i >= b) bits |= BIT_1;
		if (i >= c) bits |= BIT_2;
		if (i >= d) bits |= BIT_3;

		mPORTESetBits (bits);
	}
}

int main (void)
{
	int a, b, c, d;

	/* Configure PB frequency and wait states */
/*	SYSTEMConfigPerformance(80000000L);*/

	PORTSetPinsDigitalOut (IOPORT_E, BIT_0 | BIT_1 | BIT_2 | BIT_3);

	a = b = c = d = 0;
	for (;;) {
		pwm (a, b, c, d);
		a += 10;
		b += 100;
		c += 1000;
		d += 100000;
	}
	return 0;
}
