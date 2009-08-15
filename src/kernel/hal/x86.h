//
// x86.h
//
// Definitions of Intel processor registers and bits.
//

#ifndef _X86_H
#define _X86_H


//
// EFLAGS bit definitions
//
#define EFLAGS_IF		0x00000200	// Interrupt enable flag


//
// CR0 bit definitions
//
#define CR0_PG			0x80000000	// Enable paging
#define CR0_CD			0x40000000	// Cache disable
#define CR0_NW			0x20000000	// Not write-through (i.e., writeback)
#define CR0_WP			0x00010000	// Write protect


//
// CR4 bit definitions
//
#define CR4_PSE			0x00000010	// Enable page-size extensions
#define CR4_PGE			0x00000080	// Enable global pages


#endif
