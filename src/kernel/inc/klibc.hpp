//
// klibc.hpp
//
// Most of the standard C routines are implemented in the common user/kernel
// library.  Extra routines defined here are kernel-specific.
//

#ifndef _KLIBC_HPP
#define _KLIBC_HPP

#include "dx/types.h"
#include "stdarg.h"
#include "stdlib.h"		// itoa(), rand(), srand(), etc
#include "stdio.h"		// snprintf(), vsnprint(), etc
#include "string.h"		// memcpy(), memset(), etc


uint32_t
rand(uint32_t max);


#endif
