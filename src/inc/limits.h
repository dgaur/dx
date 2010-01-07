//
// limits.h
//

#ifndef _LIMITS_H
#define _LIMITS_H

// number of bits for smallest object that is not a bit-field (byte)
#define CHAR_BIT 8

// minimum value for an object of type signed char
#define SCHAR_MIN -127 // -(2^7 - 1)

// maximum value for an object of type signed char
#define SCHAR_MAX +127 // 2^7 - 1

// maximum value for an object of type unsigned char
#define UCHAR_MAX 255 // 2^8 - 1

// minimum value for an object of type char
#define CHAR_MIN 0		// Assume unsigned characters

// maximum value for an object of type char
#define CHAR_MAX UCHAR_MAX	// Assume unsigned characters

// maximum number of bytes in a multibyte character, for any supported locale
#define MB_LEN_MAX 1

// minimum value for an object of type short int
#define SHRT_MIN -32767 // -(2^15 - 1)

// maximum value for an object of type short int
#define SHRT_MAX +32767 // 2^15 - 1

// maximum value for an object of type unsigned short int
#define USHRT_MAX 65535 // 2^16 - 1

// minimum value for an object of type int
#define INT_MIN -2147483647 // -(2^31 - 1)

// maximum value for an object of type int
#define INT_MAX +2147483647 // 2^31 - 1

// maximum value for an object of type unsigned int
#define UINT_MAX 4294967295 // 2^32 - 1

// minimum value for an object of type long int
#define LONG_MIN -2147483647L // -(2^31 - 1)

// maximum value for an object of type long int
#define LONG_MAX +2147483647L // 2^31 - 1

// maximum value for an object of type unsigned long int
#define ULONG_MAX 4294967295UL // 2^32 - 1

// minimum value for an object of type long long int
#define LLONG_MIN -9223372036854775807LL // -(2^63 - 1)

// maximum value for an object of type long long int
#define LLONG_MAX +9223372036854775807LL // 2^63 - 1

// maximum value for an object of type unsigned long long int
#define ULLONG_MAX 18446744073709551615ULL // 2^64 - 1


#endif
