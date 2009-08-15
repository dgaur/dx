//
// ramdisk.hpp
//

#ifndef _RAMDISK_HPP
#define _RAMDISK_HPP

#include "dx/types.h"


struct  ramdisk_s;
typedef ramdisk_s *    ramdisk_sp;
typedef ramdisk_sp *   ramdisk_spp;
typedef ramdisk_s &    ramdisk_sr;
struct  ramdisk_s
	{
	const void_tp		start;
	const size_t		size;

	ramdisk_s(	const void_tp ramdisk_start,
				const void_tp ramdisk_end):
		start(ramdisk_start),
		size(uint8_tp(ramdisk_end) - uint8_tp(ramdisk_start))
		{ return; }
	};


#endif
