/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
 *
 * The contents of this file are subject to the Netscape Public
 * License Version 1.1 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy of
 * the License at http://www.mozilla.org/NPL/
 *
 * Software distributed under the License is distributed on an "AS
 * IS" basis, WITHOUT WARRANTY OF ANY KIND, either express oqr
 * implied. See the License for the specific language governing
 * rights and limitations under the License.
 *
 * The Original Code is the JavaScript 2 Prototype.
 *
 * The Initial Developer of the Original Code is Netscape
 * Communications Corporation.  Portions created by Netscape are
 * Copyright (C) 1998 Netscape Communications Corporation. All
 * Rights Reserved.
 *
 * Contributor(s): 
 *
 * Alternatively, the contents of this file may be used under the
 * terms of the GNU Public License (the "GPL"), in which case the
 * provisions of the GPL are applicable instead of those above.
 * If you wish to allow use of your version of this file only
 * under the terms of the GPL and not to allow others to use your
 * version of this file under the NPL, indicate your decision by
 * deleting the provisions above and replace them with the notice
 * and other provisions required by the GPL.  If you do not delete
 * the provisions above, a recipient may use your version of this
 * file under either the NPL or the GPL.
 */

#ifndef systemtypes_h
#define systemtypes_h

#include <cstddef>

#ifdef _WIN32
 // Turn off warnings about identifiers too long in browser information
 #pragma warning(disable: 4786)
#endif

// Define int8, int16, int32, int64, uint8, uint16, uint32, uint64, and uint.
typedef unsigned int uint;
typedef unsigned char uchar;

typedef signed char int8;
typedef unsigned char uint8;
typedef short int16;
typedef unsigned short uint16;
#if !defined(XP_MAC) && !defined(_WIN32)
 typedef int int32;
 typedef unsigned int uint32;
#else
 typedef long int32;
 typedef unsigned long uint32;
#endif
#ifdef _WIN32
 typedef __int64 int64;
 typedef unsigned __int64 uint64;
#else
 typedef long long int64;
 typedef unsigned long long uint64;
#endif

// Define this if the machine natively supports 64-bit integers
#define NATIVE_INT64

// Define float32 and float64.
typedef double float64;
typedef float float32;

// A UTF-16 character
// Use wchar_t on platforms on which wchar_t has 16 bits; otherwise use int16.
// Note that in C++ wchar_t is a distinct type rather than a typedef for some integral type.
// Like char, a char16 can be either signed or unsigned at the implementation's discretion.
#ifdef __GNUC__
 // GCC's wchar_t is 32 bits, so we can't use it.
 typedef uint16 char16;
 typedef uint16 uchar16;
#else
 typedef wchar_t char16;
 #ifndef _WIN32 // Microsoft VC6 bug: wchar_t should be a built-in type, not a typedef
  typedef wchar_t uchar16;
 #else
  typedef wchar_t uchar16;
 #endif
#endif

#ifdef _WIN32
 #define IS_LITTLE_ENDIAN
#endif
#ifdef __i386__
 #define IS_LITTLE_ENDIAN
#endif


// basicAlignment is the maximum alignment required by any native type.  An
// object aligned to a multiple of basicAlignment can hold any native type.
// malloc should return a pointer whose lgBasicAlignment least significant bits
// are clear. Currently basicAlignment is set to 8 to allow doubles and int64s
// to have their natural alignment; may be customized for individual platforms.
const uint lgBasicAlignment = 3;
const uint basicAlignment = 1u<<lgBasicAlignment;

//
// Bit manipulation
//

#define JS_BIT(n)       ((uint32)1 << (n))
#define JS_BITMASK(n)   (JS_BIT(n) - 1)

#endif /* systemtypes_h */
