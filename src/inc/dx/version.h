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
#define DX_VERSION         "0.0.7"
#define DX_RELEASE_DATE    "2011-11-19"



//
// Build type (production/retail or debug)
//
#ifdef DEBUG
#define		DX_BUILD_TYPE		"Debug"
#else
#define		DX_BUILD_TYPE		"Production"
#endif


#endif
