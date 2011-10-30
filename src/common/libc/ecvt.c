//
// ecvt.c
//

#include "stdlib.h"


//
// dtoa() in libgdtoa
//
extern char*
dtoa(double d, int mode, int ndigits, int *decpt, int *sign, char **rve);


char* ecvt(double number, int ndigits, int *decpt, int *sign)
	{
	return(dtoa(number, 2, ndigits, decpt, sign, NULL));
	}


char* fcvt(double number, int ndigits, int *decpt, int *sign)
	{
	return(dtoa(number, 3, ndigits, decpt, sign, NULL));
	}
