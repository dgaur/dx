//
// bits.asm
//
// Miscellaneous logic for manipulating bits and bit-fields.  The portions
// here rely on specific x86 instructions/functionality
//


.text



//
// find_one_bit32()
//
// Find the lowest set/one bit within a 32-bit value.  No side effects.
// Returns a value in the range 0 - 31 on success, or 0xFFFFFFFF if the input
// value was initially zero.
//
// find_one_bit32(0x0)	=> 0xFFFFFFFF
// find_one_bit32(0x1)	=> 0
// find_one_bit32(0x2)	=> 1
// find_one_bit32(0x3)	=> 0
// find_one_bit32(0x4)	=> 2
//
// C/C++ prototype --
//		uint32_t
//		find_one_bit32(uint32_t value);
//
.align 4
.global find_one_bit32
find_one_bit32:
	// Load the input value into %eax
	movl 4(%esp), %eax

	// Find the lowest-order set bit, if any
	bsfl %eax, %eax

	// Was the input already zero?
	jnz 1f
	movl $0xFFFFFFFF, %eax

1:
	ret



//
// find_zero_bit32()
//
// Find the lowest clear/zero bit within a 32-bit value.  No side effects.
// Returns an index in the range 0 - 31 on success, or 0xFFFFFFFF if the
// input value was initially 0xFFFFFFFF.
//
// find_zero_bit32(0x0)	=> 0
// find_zero_bit32(0x1)	=> 1
// find_zero_bit32(0x2)	=> 0
// find_zero_bit32(0x3)	=> 2
// find_zero_bit32(0x5)	=> 1
//
// C/C++ prototype --
//		uint32_t
//		find_zero_bit32(uint32_t value);
//
.align 4
.global find_zero_bit32
find_zero_bit32:
	// Load the input value into %eax
	movl 4(%esp), %eax

	// Find the lowest-order zero bit, if any
	notl %eax
	bsfl %eax, %eax

	// Was the input already 0xFFFFFFFF?
	jnz 1f
	movl $0xFFFFFFFF, %eax

1:
	ret


