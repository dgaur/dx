//
// multiboot.hpp
//
// Definitions and structures emitted by the Multiboot loader (GRUB).  The
// boot loader provides this data to the kernel to describe the physical
// host, etc.
//

#ifndef _MULTIBOOT_HPP
#define _MULTIBOOT_HPP

#include "dx/types.h"


///
/// Magic number used to identify a Multiboot-compliant loader
///
const
uint32_t	MULTIBOOT_BOOTLOADER_MAGIC_NUMBER	= 0x2BADB002;



#pragma pack(1)

///
/// Data about interfacing with the APM BIOS, if any
///
struct  apm_interface_s;
typedef apm_interface_s *    apm_interface_sp;
typedef apm_interface_sp *   apm_interface_spp;
typedef apm_interface_s &    apm_interface_sr;
struct  apm_interface_s
	{
	uint16_t	version;
	uint16_t	code_segment32;
	uint32_t	entry_point;
	uint16_t	code_segment16;
	uint16_t	data_segment16;
	uint16_t	flags;
	uint16_t	code_segment32Size;
	uint16_t	code_segment16Size;
	uint16_t	data_segment16Size;
	};



///
/// Data about the boot disk
///
struct  boot_device_s;
typedef boot_device_s *    boot_device_sp;
typedef boot_device_sp *   boot_device_spp;
typedef boot_device_s &    boot_device_sr;
struct  boot_device_s
	{
	uint8_t		drive;
	uint8_t		partition1;
	uint8_t		partition2;
	uint8_t		partition3;
	};


typedef enum
	{
	DISK_DRIVE_MODE_CHS	= 0,	// Cylinder/head/sector addressing
	DISK_DRIVE_MODE_LBA = 1		// Logical block addressing
	} disk_drive_mode_e;



///
/// Data about a physical disk drive
///
struct  disk_drive_s;
typedef disk_drive_s *    disk_drive_sp;
typedef disk_drive_sp *   disk_drive_spp;
typedef disk_drive_s &    disk_drive_sr;
struct  disk_drive_s
	{
	uint32_t			size;
	uint8_t				index;
	disk_drive_mode_e	mode;

	// Physical disk geometry
	uint16_t			cylinder_count;
	uint8_t				head_count;
	uint8_t				sector_count;

	// Array of I/O ports
	uint16_t			port[0];
	};



///
/// Symbol table for the kernal ELF image
///
struct  elf_symbol_table_s;
typedef elf_symbol_table_s *    elf_symbol_table_sp;
typedef elf_symbol_table_sp *   elf_symbol_table_spp;
typedef elf_symbol_table_s &    elf_symbol_table_sr;
struct  elf_symbol_table_s
	{
	uint32_t	count;
	uint32_t	size;
	uint32_t	address;
	uint32_t	shndx; //@
	};



///
/// Data about a region of physical memory.  This is essentially the
/// structure returned from BIOS interrupt 0x15 (function 0xe820), plus
/// the additional Size field.
///
struct  memory_map_s;
typedef memory_map_s *    memory_map_sp;
typedef memory_map_sp *   memory_map_spp;
typedef memory_map_s &    memory_map_sr;
struct  memory_map_s
	{
	uint32_t	size;	// Size of the structure, not including Size field
	uint64_t	base_address;
	uint64_t	length;
	uint32_t	type;
	// ... possibly additional, opaque BIOS data here
	};

const
uint32_t	MEMORY_MAP_TYPE_AVAILABLE_MEMORY	= 1,	// RAM available to OS
			MEMORY_MAP_TYPE_RESERVED			= 2,
			MEMORY_MAP_TYPE_ACPI_RECLAIM_MEMORY	= 3,
			MEMORY_MAP_TYPE_ACPI_NVS_MEMORY		= 4;



///
/// Data about an additional module loaded by the boot loader
///
struct  module_data_s;
typedef module_data_s *    module_data_sp;
typedef module_data_sp *   module_data_spp;
typedef module_data_s &    module_data_sr;
struct  module_data_s
	{
	void_tp		start_address;
	void_tp		end_address;
	char8_tp	name;
	uint32_t	reserved;
	};



///
/// Data about the video support
///
struct  video_data_s;
typedef video_data_s *    video_data_sp;
typedef video_data_sp *   video_data_spp;
typedef video_data_s &    video_data_sr;
struct  video_data_s
	{
	uint32_t	control;
	uint32_t	mode_data;
	uint32_t	mode;
	uint32_t	interface_segment;
	uint32_t	interface_offset;
	uint32_t	interface_size;
	};



///
/// The main Multiboot data structure, delivered to the kernel by
/// the boot loader
///
struct  multiboot_data_s;
typedef multiboot_data_s *    multiboot_data_sp;
typedef multiboot_data_sp *   multiboot_data_spp;
typedef multiboot_data_s &    multiboot_data_sr;
struct  multiboot_data_s
	{
	uint32_t			flags;

	uint32_t			low_memory_size;	// Measured in KB
	uint32_t			high_memory_size;	// Measured in KB

	boot_device_s		boot_device;
	char8_tp			command_line;

	// Additional modules loaded by the boot loader
	uint32_t			module_count;
	module_data_sp		module_data;

	elf_symbol_table_s	symbol_table;

	// Memory map
	uint32_t			memory_map_size;
	memory_map_sp		memory_map;

	// Physical disk datarmation
	uint32_t			disk_drive_size;
	disk_drive_sp		disk_drive;

	uint32_tp			bios_configuration; //@need this structure
	char8_tp			boot_loader_name;
	apm_interface_s		apm;
	video_data_s		video;
	};

#pragma pack()


//
// These flags indicate which of the fields in the multiboot_data_s structure
// are populated/valid
//
const
uint32_t	MULTIBOOT_DATA_MEMORY_SIZES			= 0x001,
			MULTIBOOT_DATA_BOOT_DEVICE			= 0x002,
			MULTIBOOT_DATA_COMMAND_LINE			= 0x004,
			MULTIBOOT_DATA_MODULE_DATA			= 0x008,
			MULTIBOOT_DATA_AOUT_SYMBOL_TABLE	= 0x010,
			MULTIBOOT_DATA_ELF_SYMBOL_TABLE		= 0x020,
			MULTIBOOT_DATA_MEMORY_MAP			= 0x040,
			MULTIBOOT_DATA_DISK_DRIVE			= 0x080,
			MULTIBOOT_DATA_BIOS_CONFIG			= 0x100,
			MULTIBOOT_DATA_APM_INTERFACE		= 0x200,
			MULTIBOOT_DATA_VIDEO_DATA			= 0x400;



///
/// Global copy of the Multiboot structure, available to any kernel subsystem
/// or driver that might want it
///
extern
multiboot_data_sp	__multiboot_data;


#endif


