//
// read.h
//

#ifndef _READ_H
#define _READ_H

#include "stream.h"

size_t
maybe_read(FILE* stream, void* buffer, size_t buffer_size);

size_t
read(FILE* stream, void* buffer, size_t buffer_size);

#endif

