// orb - HTML preprocessor
// errrpt.c - error-reporting functions
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


#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include "errrpt.h"

static ErrLevel minLevel = Info;

static const char* label[Always + 1] = {
  NULL, NULL, "Warning", "Error", "Fatal error", NULL
};

void perr(ErrLevel lev, const char* fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);

  if (lev >= minLevel) {
    if (label[lev]) {
      fprintf(stderr, "%s: ", label[lev]);
    }

    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
  }

  if (lev == Die) exit(EXIT_FAILURE);

  va_end(ap);
}

void perr_fl(const char* inName, unsigned lineNum,
             ErrLevel lev, const char* fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);

  if (lev >= minLevel) {
    fprintf(stderr, "(%s:%u) ", inName, lineNum);

    if (label[lev]) {
      fprintf(stderr, "%s: ", label[lev]);
    }

    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
  }

  if (lev == Die) exit(EXIT_FAILURE);

  va_end(ap);
}

void setMinLevel(ErrLevel lev)
{
  minLevel = lev;
}
