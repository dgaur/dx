//
// dx/thread_id.h
//

#ifndef _THREAD_ID_H
#define _THREAD_ID_H

#include "stdint.h"


//
// Each thread is known by a unique, numeric id.  No two threads will share
// the same id simultaneously, even across multiple processes
//
typedef	uintptr_t			thread_id_t;
typedef thread_id_t*		thread_id_tp;
typedef thread_id_tp*		thread_id_tpp;



//
// Some well-known thread ids
//
#define THREAD_ID_NULL		((thread_id_t)(-256))	//@SMP one per CPU

#define THREAD_ID_BOOT		((thread_id_t)(-16))	// 0xFFFFFFF0
#define THREAD_ID_CLEANUP	((thread_id_t)(-15))	// 0xFFFFFFF1, etc

#define THREAD_ID_LOOPBACK	((thread_id_t)(-2))		/// Loopback to self
#define THREAD_ID_INVALID	((thread_id_t)(-1))



#endif
