//
// atomic_int32.hpp
//
// Integer type that supports atomic increment/decrement.  Useful
// for reference counters, etc, where full locks are not required.
//

#ifndef _ATOMIC_INT32_HPP
#define _ATOMIC_INT32_HPP

#include "dx/types.h"


class   atomic_int32_c;
typedef atomic_int32_c *    atomic_int32_cp;
typedef atomic_int32_cp *   atomic_int32_cpp;
typedef atomic_int32_c &    atomic_int32_cr;
class   atomic_int32_c
	{
	private:
		volatile int32_t	value;

	protected:

	public:
		atomic_int32_c(int32_t initial_value = 0)
			{ *this = initial_value; return; }
		~atomic_int32_c()
			{ return; }


		//
		// Read/write operations
		//
		atomic_int32_cr
			operator=(int32_t new_value);
		operator
			int32_t() const;


		//
		// Increment operations
		//
		inline
		atomic_int32_cr
			operator++()		// prefix operator
				{ (*this)++; return(*this); }

		atomic_int32_cr
			operator++(int);	// postfix operator

		inline
		void_t
			increment()
				{ (*this)++; return; }

		int32_t
			increment_and_read();


		//
		// Decrement operations
		//
		inline
		atomic_int32_cr
			operator--()		// prefix operator
				{ (*this)--; return(*this); }

		atomic_int32_cr
			operator--(int);	// postfix operator

		inline
		void_t
			decrement()
				{ (*this)--; return; }

		int32_t
			decrement_and_read();

	};



#endif
