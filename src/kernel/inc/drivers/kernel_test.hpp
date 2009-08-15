//
// kernel_test.hpp
//
// Driver for kernel testing.  Exercises various kernel functions +
// features; a quasi-unittest for the kernel.
//

#ifndef _KERNEL_TEST_HPP
#define _KERNEL_TEST_HPP

#include "dx/types.h"


class   kernel_test_c;
typedef kernel_test_c *    kernel_test_cp;
typedef kernel_test_cp *   kernel_test_cpp;
typedef kernel_test_c &    kernel_test_cr;
class   kernel_test_c
	{
	private:

	protected:

	public:
		kernel_test_c()
			{ return; }
		~kernel_test_c()
			{ return; }

		uint32_t
			run_tests();
	};


#endif
