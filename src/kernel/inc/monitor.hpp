//
// monitor.hpp
//

#ifndef _MONITOR_HPP
#define _MONITOR_HPP

#include "dx/types.h"
#include "interrupt.hpp"



///
/// Kernel monitor.  Reports kernel stats and other system data out to
/// user space.
///
class   kernel_monitor_c;
typedef kernel_monitor_c *    kernel_monitor_cp;
typedef kernel_monitor_cp *   kernel_monitor_cpp;
typedef kernel_monitor_c &    kernel_monitor_cr;
class   kernel_monitor_c
	{
	private:

	protected:

	public:
		kernel_monitor_c()
			{ return; }
		~kernel_monitor_c()
			{ return; }

		static
		void_t
			handle_interrupt(interrupt_cr interrupt);
	};


#endif

