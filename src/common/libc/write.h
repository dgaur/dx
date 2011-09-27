//
// write.h
//

#ifndef _WRITE_H
#define _WRITE_H

#include "stream.h"

size_t
maybe_write(FILE* stream, const void* data, size_t data_size);

size_t
write(FILE* stream, const void* data, size_t data_size);


#endif

