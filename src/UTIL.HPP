// orb - HTML preprocessor
// util.hpp - utilities header
//
// Copyright (C) 1996-1998  Craig Berry
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,
// USA.


#ifndef UTIL_HPP
#define UTIL_HPP

#include <ctype.h>
#include <stddef.h>
#include <string.h>

enum CaseMode { Lower, Upper, Mixed };

/* Configuration parameters and local types. */

#ifdef __MSDOS__
  #include <dir.h>
  #define CASEMODE Lower
#else
  #define MAXPATH  200
  #define CASEMODE Mixed
#endif

#if defined(__MSDOS__) || defined(__OS2__)
  #define DIRSEP   "\\"
  #define ARGSEP   ";"
#else
  #define DIRSEP   "/"
  #define ARGSEP   ":"
#endif

const size_t      maxExtUsed         = 10;
const char* const startMark          = "[[";
const char* const endMark            = "]]";
const size_t      startMarkLen       = strlen(startMark);
const size_t      endMarkLen         = strlen(endMark);

/* Values which can be changed through cmd-line options. */

extern int        debugFlag;
extern int        lineSpliceFlag;
extern int        bashfulFlag;
extern char       outExt[maxExtUsed];
extern char       outPath[MAXPATH];
extern CaseMode   caseMode;

/* Storage for include-file search paths. */

const  size_t     maxInclPath        = 20;
extern size_t     inclPathCount;
extern size_t     inclPathCountCmd;
extern char       inclPath[maxInclPath][MAXPATH];

/* Utility functions. */

void strFixCase(char* str);

#endif
