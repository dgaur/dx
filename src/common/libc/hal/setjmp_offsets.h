//
// setjmp_offsets.h
//

#ifndef _SETJMP_OFFSETS_H
#define _SETJMP_OFFSETS_H


//
// Offsets into the jmp_buf array.  No need to save EAX, since it's overwritten
// on every invocation
//
#define EBX_OFFSET		0x00
#define ECX_OFFSET		0x04
#define EDX_OFFSET		0x08
#define ESI_OFFSET		0x0C
#define EDI_OFFSET		0x10
#define EBP_OFFSET		0x14
#define ESP_OFFSET		0x18
#define EIP_OFFSET		0x1C

#endif
