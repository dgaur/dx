//
// elf.h
//

#ifndef _ELF_H
#define _ELF_H

#include "dx/types.h"


//
// Types/aliases used by the ELF standard
//
typedef uint32_t	elf_address_t;
typedef uint16_t	elf_half_t;
typedef uint32_t	elf_offset_t;
typedef int32_t		elf_sword_t;
typedef uint32_t	elf_word_t;


#define ELF_IDENT_SIZE 16


///
/// ELF file header.  Describes the layout + contents of the entire ELF file
///
typedef struct elf_header
	{
	char8_t			ident[ ELF_IDENT_SIZE ];
	elf_half_t		type;
	elf_half_t		machine;
	elf_word_t		version;
	elf_address_t	entry;
	elf_offset_t	program_header_offset;
	elf_offset_t	section_header_offset;
	elf_word_t		flags;
	elf_half_t		header_size;
	elf_half_t		program_header_entry_size;
	elf_half_t		program_header_entry_count;
	elf_half_t		section_header_entry_size;
	elf_half_t		section_header_entry_count;
	elf_half_t		section_header_string_index;
	} elf_header_s;
typedef elf_header_s*	elf_header_sp;
typedef elf_header_sp*	elf_header_spp;


///
/// The magic string/cookie that marks a binary file as an ELF image
///
#define ELF_IDENT_MAGIC			"\x7F" "ELF"


#define ELF_TYPE_NONE			0
#define ELF_TYPE_RELOCATABLE	1
#define ELF_TYPE_EXECUTABLE		2
#define ELF_TYPE_DYNAMIC		3
#define ELF_TYPE_CORE			4

#define ELF_MACHINE_NONE		0
#define ELF_MACHINE_M32			1
#define ELF_MACHINE_SPARC		2
#define ELF_MACHINE_386			3
#define ELF_MACHINE_68K			4
#define ELF_MACHINE_88K			5
#define ELF_MACHINE_860			7
#define ELF_MACHINE_MIPS		8




///
/// ELF Program Header.  Describes the layout + contents of one program segment
///
typedef struct elf_program_header
	{
	elf_word_t		type;
	elf_offset_t	offset;
	elf_address_t	virtual_address;
	elf_address_t	physical_address;
	elf_word_t		file_size;
	elf_word_t		memory_size;
	elf_word_t		flags;
	elf_word_t		align;
	} elf_program_header_s;
typedef elf_program_header_s*		elf_program_header_sp;
typedef elf_program_header_sp*		elf_program_header_spp;


#define ELF_PROGRAM_TYPE_NULL		0
#define ELF_PROGRAM_TYPE_LOAD		1	// .text, .data, etc
#define ELF_PROGRAM_TYPE_DYNAMIC	2
#define ELF_PROGRAM_TYPE_INTERP		3
#define ELF_PROGRAM_TYPE_NOTE		4
#define ELF_PROGRAM_TYPE_SHLIB		5
#define ELF_PROGRAM_TYPE_PHDR		6



#endif
