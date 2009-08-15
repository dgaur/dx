//
// unistd.h
//

#ifndef _UNISTD_H
#define _UNISTD_H

#include "stdint.h"


// To support malloc().  This is considered a "legacy" routine in the POSIX
// specs, and was removed altogether in POSIX.3
void*
sbrk(intptr_t delta);


#endif
