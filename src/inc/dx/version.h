//
// version.h
//
// DX version information.  The make_release.sh script updates this file
// automatically
//

#ifndef _VERSION_H
#define _VERSION_H



//
// Current version number
//
#define DX_VERSION         "post-0.0.2"
#define DX_RELEASE_DATE    "unreleased"



//
// Build type (production/retail or debug)
//
#ifdef DEBUG
#define		DX_BUILD_TYPE		"Debug"
#else
#define		DX_BUILD_TYPE		"Production"
#endif


#endif
