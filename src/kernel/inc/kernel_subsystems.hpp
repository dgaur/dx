//
// kernel_subsystems.hpp
//
// The entire list of global kernel subsystems.  These components
// are available anywhere in the kernel.  All subsystems are
// initialized at boot-time in kernel_init().
//

#ifndef _KERNEL_SUBSYSTEMS_HPP
#define _KERNEL_SUBSYSTEMS_HPP

#include "device_proxy.hpp"
#include "hal/x86_hal.hpp"
#include "io_manager.hpp"
#include "memory_manager.hpp"
#include "thread_manager.hpp"


//
// Each of these pointers is declared in the .cpp file that implements
// the component in question.
//

extern
device_proxy_cp							__device_proxy;

extern
x86_hardware_abstraction_layer_cp		__hal;

extern
io_manager_cp							__io_manager;

extern
memory_manager_cp						__memory_manager;

extern
thread_manager_cp						__thread_manager;

#endif

