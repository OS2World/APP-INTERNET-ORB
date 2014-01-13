// orb - HTML preprocessor
// util.cc - utilities and misc
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


#include "util.hpp"

// Values which can be changed through cmd-line options.

int        debugFlag          = 0;
int        lineSpliceFlag     = 1;
int        bashfulFlag        = 0;
char       outExt[maxExtUsed] = ".html";
char       outPath[MAXPATH]   = "";
CaseMode   caseMode           = Mixed;

// Storage for include-file search paths.

size_t     inclPathCount      = 1;
size_t     inclPathCountCmd   = 0;
char       inclPath[maxInclPath][MAXPATH] = { "." };

// Utility functions.

void strFixCase(char* str)
{
  if (caseMode == Lower) {
    for (char* p = str; *p; p++) *p = tolower(*p);
  }
  else if (caseMode == Upper) {
    for (char* p = str; *p; p++) *p = toupper(*p);
  }
}
