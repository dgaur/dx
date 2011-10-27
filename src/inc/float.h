//
// float.h
//
// Floating-point limits
//

#ifndef _FLOAT_H
#define _FLOAT_H


//
// The values here should all be consistent with IEEE 854 --
//
// Floating-point values are represented as 1.f * (b ^ (e - ebias)), where:
//	f = fractional portion, < 1.0
//	b = base = 2
//	e = exponent
//	ebias = exponent bias
//	p = precision = length(f) + 1, in bits
//
//						size (bits):	precision p (bits):		
// Single precision:	32				24
// Double precision:	64				53
// Extended precision:	80				64
//
// Represented as:
//	- one bit of sign (S);
//	- M bits of exponent (E);
//	- (p-1) bits of fraction (F);
//	= SEEE...EEEFFFFF....FFFF
//


//
// The rounding mode for floating-point addition is characterized by the
// implementation-defined value of FLT_ROUNDS:
//  -1 indeterminable
//   0 toward zero
//   1 to nearest
//   2 toward positive infinity
//   3 toward negative infinity
// All other values for FLT_ROUNDS characterize implementation-defined rounding
// behavior.
//
#define FLT_ROUNDS 1		//@default for x86 FPU control word


//
// The use of evaluation formats is characterized by the
// implementation-defined value of FLT_EVAL_METHOD:
//  -1 indeterminable;
//   0 evaluate all operations and constants just to the range and precision of
//     the type;
//   1 evaluate operations and constants of type float and double to the range
//     and precision of the double type, evaluate long double operations and
//     constants to the range and precision of the long double type;
//   2 evaluate all operations and constants to the range and precision of the
//     long double type.
// All other negative values for FLT_EVAL_METHOD characterize
// implementation-defined behavior.
//
#define FLT_EVAL_METHOD	0	//@


//
// Radix of exponent representation, b
//
#define FLT_RADIX	2


//
// Number of base-FLT_RADIX digits in the floating-point significand, p
//
#define FLT_MANT_DIG	(23 + 1)	// Explicit fraction + implied integer
#define DBL_MANT_DIG	(52 + 1)	// Explicit fraction + implied integer
#define LDBL_MANT_DIG	(63 + 1)	// Explicit fraction + implied integer


//
// Number of decimal digits, n, such that any floating-point number in
// the widest supported floating type with pmax radix b digits can be
// rounded to a floating-point number with n decimal digits and back
// again without change to the value,
//	(pmax * log10 b), if b is a power of 10
//	ceil(1 + pmax * log10 b), otherwise
//
#define DECIMAL_DIG	21		// pmax = 64 bits for long double


//
// Number of decimal digits, q, such that any floating-point number with q
// decimal digits can be rounded into a floating-point number with p radix b
// digits and back again without change to the q decimal digits,
//	(p * log10 b), if b is a power of 10
//	floor( (p-1) * log10 b), otherwise
//
#define FLT_DIG		6
#define DBL_DIG		15
#define LDBL_DIG	18


//
// Minimum negative integer such that FLT_RADIX raised to one less
// than that power is a normalized floating-point number, emin
//
#define FLT_MIN_EXP		(-126 + 1)
#define DBL_MIN_EXP		(-1022 + 1)
#define LDBL_MIN_EXP	(-16382 + 1)


//
// Minimum negative integer such that 10 raised to that power is in
// the range of normalized floating-point numbers,
//   ceil(log10 b^(emin - 1))
//
#define FLT_MIN_10_EXP		(-38)
#define DBL_MIN_10_EXP		(-308)
#define LDBL_MIN_10_EXP		(-4932)


//
// Maximum integer such that FLT_RADIX raised to one less than that
// power is a representable finite floating-point number, emax
//
#define FLT_MAX_EXP		(127 + 1)
#define DBL_MAX_EXP		(1023 + 1)
#define LDBL_MAX_EXP	(16383 + 1)


//
// Maximum integer such that 10 raised to that power is in the range
// of representable finite floating-point numbers,
//	ceil( log10((1 - b^-p) * b^emax) )
//
#define FLT_MAX_10_EXP		(38)
#define DBL_MAX_10_EXP		(308)
#define LDBL_MAX_10_EXP		(4932)


//
// Maximum representable finite floating-point number, (1 - b^-p) * b^emax
//
#define FLT_MAX		3.40E+38
#define DBL_MAX		1.79E+308
#define LDBL_MAX	1.18E+4932


//
// The difference between 1 and the least value greater than 1 that is
// representable in the given floating point type, b^(1-p)
//
#define FLT_EPSILON		1.19209289551e-7
#define DBL_EPSILON		2.22044604925e-16
#define LDBL_EPSILON	1.08420217249e-19



//
// Minimum normalized positive floating-point number, b^(emin-1)
//
#define FLT_MIN		1.18E-38
#define DBL_MIN		2.23E-308
#define LDBL_MIN	3.37E-4932



#endif

