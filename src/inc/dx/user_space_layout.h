//
// dx/user_space_layout.h
//

#ifndef _USER_SPACE_LAYOUT_H
#define _USER_SPACE_LAYOUT_H


///
/// The user/kernel threshold.  Linear addresses above this value are user
/// space; below this value are kernel space
///
#define	USER_KERNEL_BOUNDARY	0x40000000	// 1GB


///
/// Environment descriptor.  Contains various load-time and run-time
/// settings (e.g., command line arguments, heap pointers, etc) for the current
/// address space.
///
#define USER_ENVIRONMENT_BLOCK	0xFFC00000	// Uppermost 4MB of address space


#endif
