//
// types.h
//
// dx-specific types.  These are specific to the dx libraries + kernel, not
// part of C99 or any other standard
//

#ifndef _TYPES_H
#define _TYPES_H



//
// Inherit the standard C (C99) types + symbols
//
#include "stddef.h"		// size_t and NULL
#include "stdint.h"		// Standard C99 types
#include "stdbool.h"	// Standard C99 boolean type



//
// Augment the standard types with some extra type's and typedef's for
// consistency
//

typedef char				bool_t;
typedef bool_t *			bool_tp;
typedef bool_tp *			bool_tpp;

#define TRUE	true
#define FALSE	false

typedef	char				char8_t;
typedef	char8_t	*			char8_tp;
typedef	char8_tp *			char8_tpp;

typedef	short				char16_t;
typedef	char16_t *			char16_tp;
typedef	char16_tp *			char16_tpp;

typedef int8_t *			int8_tp;
typedef int8_tp *			int8_tpp;

typedef int16_t *			int16_tp;
typedef int16_tp *			int16_tpp;

typedef int32_t *			int32_tp;
typedef int32_tp *			int32_tpp;

typedef int64_t *			int64_tp;
typedef int64_tp *			int64_tpp;

typedef intptr_t *			intptr_tp;
typedef intptr_tp *			intptr_tpp;

typedef uint8_t *			uint8_tp;
typedef uint8_tp *			uint8_tpp;

typedef uint16_t *			uint16_tp;
typedef uint16_tp *			uint16_tpp;

typedef uint32_t *			uint32_tp;
typedef uint32_tp *			uint32_tpp;

typedef uint64_t *			uint64_tp;
typedef uint64_tp *			uint64_tpp;

typedef uintptr_t *			uintptr_tp;
typedef uintptr_tp *		uintptr_tpp;

typedef void				void_t;
typedef void_t *			void_tp;
typedef void_tp *			void_tpp;

#endif

