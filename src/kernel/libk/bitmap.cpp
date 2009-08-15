//
// bitmap.cpp
//

#include "bitmap.hpp"
#include "new.hpp"


//
// allocate_bitmap()
//
// Factory function for allocating bitmaps based on the desired size of
// the bitmap.  Returns a bitmap object that supports the requested
// number of bits, or NULL on error.
//
bitmap_cp
allocate_bitmap(uint32_t max_bits)
	{
	bitmap_cp bitmap = NULL;

	if (max_bits <= 32)
		bitmap = new bitmap32_c(max_bits);
	else if (max_bits <= 1024)
		bitmap = new bitmap1024_c(max_bits);
	else if (max_bits <= 32*1024)
		bitmap = new bitmap32k_c(max_bits);

	return(bitmap);
	}

