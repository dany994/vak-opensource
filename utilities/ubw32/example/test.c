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

#include <p32xxxx.h>
#include <plib.h>
#include <math.h>

#define DEG2RAD (M_PI / 180.0)

static void pwm(
  const short a,
  const short b,
  const short c,
  const short d)
{
	int   bits;
	short i;

	bits = a ? BIT_0 : 0;
	if(b > 0) bits |= BIT_1;
	if(c > 0) bits |= BIT_2;
	if(d > 0) bits |= BIT_3;
	mPORTEClearBits(bits);

	for(i=0;i<=1000;i++) {
		bits = (i >= a) ? BIT_0 : 0;
		if(i >= b) bits |= BIT_1;
		if(i >= c) bits |= BIT_2;
		if(i >= d) bits |= BIT_3;

		mPORTESetBits(bits);
	}
}

int main(void)
{
	double d;

	/* Configure PB frequency and wait states */
	SYSTEMConfigPerformance(80000000L);

	PORTSetPinsDigitalOut(IOPORT_E,BIT_0 | BIT_1 | BIT_2 | BIT_3);

	for(d = 0.0;;d += 0.005) {
	  pwm((int)(pow(0.5 + cos(d                  ) * 0.5,3.0) * 1000.0),
	      (int)(pow(0.5 + cos(d +  90.0 * DEG2RAD) * 0.5,3.0) * 1000.0),
	      (int)(pow(0.5 + cos(d + 180.0 * DEG2RAD) * 0.5,3.0) * 1000.0),
	      (int)(pow(0.5 + cos(d + 270.0 * DEG2RAD) * 0.5,3.0) * 1000.0));
	}

	return 0;
}
