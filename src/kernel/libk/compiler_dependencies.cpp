//
// compiler_dependencies.cpp
//

#include "dx/compiler_dependencies.h"
#include "dx/types.h"


#ifdef __GNUC__


//
// __cxa_pure_virtual()
//
// GCC requires a function with this signature when using abstract
// base classes.  If an object is somehow constructed without
// properly implementing one of its virtual methods, GCC uses this
// method as the default/fallback handler.  In general, this method
// should never be called.
//
// The signature intentionally mirrors the standard implementation
// in the libstdc++ library.
//
ASM_LINKAGE
void
__cxa_pure_virtual()
    { return; }


//@@@@@@
// Workaround for GCC issue -- explicitly specialize the bitmap template
// to ensure its code + vtable are included in the kernel image.  Otherwise,
// GCC will not emit its vtable and the linker will complain/fail.  The GCC
// documentation indicates that this issue is possibly the result of a missing
// "key method" in the template, although the addition of a key method does
// not fix the problem.  More likely, this is an issue with the configuration
// of the cross-compiler -- the native cygwin build of GCC does not exhibit
// this link error; it only manifests with the custom GCC cross-compiler.
// This obviously implies the issue is specific to the cross-compiler, although
// none of the GCC configuration options appear to have any control over this
// feature/behavior.
#include "bitmap.hpp"
template class multilevel_bitmap_m<bitmap32_c>;
template class multilevel_bitmap_m<bitmap1024_c>;

#endif
