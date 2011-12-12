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
#define DX_VERSION         "0.1.0"
#define DX_RELEASE_DATE    "2011-12-11"



//
// Build type (production/retail or debug)
//
#ifdef DEBUG
#define		DX_BUILD_TYPE		"Debug"
#else
#define		DX_BUILD_TYPE		"Production"
#endif


#endif
