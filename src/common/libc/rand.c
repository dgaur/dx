//
// rand.c
//

#include "stdint.h"
#include "stdlib.h"		// PRNG_M, PRNG_G


static
uint32_t random_seed = 1;



//
// Pseudo-random number generator.  Returns a pseudo-random number in
// the range [0 .. PRNG_M).
//
// This is the Park-Miller linear congruential PRNG.  Not suitable for
// high-grade randomness, cryptography, etc.  See also Knuth, vol 2,
// or Sedgewick.
//
// The logic here is not thread-safe, since it relies on the global seed value.
// If multiple threads invoke this routine simultaneously, the routine may
// return the same "random result" to each thread.
//
int
rand()
	{
	uint32_t result32;
	uint64_t result64;

	// Compute the next result in the series; and update the internal seed
	// for the next invocation
	result64 = (PRNG_G * (uint64_t)(random_seed)) % PRNG_M;
	random_seed = result32 = (uint32_t)(result64);

	return(result32);
	}


//
// Seeds the rand() RNG with the given value.
//
void
srand(unsigned int seed)
	{
	// The initial seed must be in the range (1 .. PRNG_M - 1)
	if (seed == 0)
		seed++;
	random_seed = (seed % PRNG_M);
	return;
	}


