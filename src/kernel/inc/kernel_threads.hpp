//
// kernel_threads.hpp
//
// The set of "important" kernel threads.  These threads are available
// throughout the kernel as needed.
//
// @See thread_manager_c::initialize_system_threads()
//

#ifndef _KERNEL_THREADS_HPP
#define _KERNEL_THREADS_HPP

#include "thread.hpp"


//
// Each of these pointers is declared in the .cpp file that implements
// that thread
//


/// The cleanup thread destroys + reclaims thread contexts
extern
thread_cp		__cleanup_thread;


/// The idle thread consumes CPU cycles when no other threads are ready to run
extern
thread_cp		__idle_thread;


/// The null thread is a message-sink; and provides a convenient source for
/// messages that originate in the kernel
extern
thread_cp		__null_thread;


#endif

