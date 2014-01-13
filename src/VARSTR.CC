// orb - HTML preprocessor
// varstr.cc - variable-length string class implementation
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


#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "varstr.hpp"

VarStr::VarStr(const char* _s, int len) :
  sz(Block),
  valFlag(1)
{
  if (_s != NULL)      sz = strlen(_s);
  if ((int) sz > len)  sz = len;
  sz++;

  assert((s = (char*) malloc(sz)) != NULL);

  if (_s != NULL)
    set(_s, len);
  else
    *s = '\0';
}

VarStr::VarStr(size_t _sz) :
  sz(_sz),
  valFlag(1)
{
  assert(sz > 0);
  assert((s = (char*) malloc(sz)) != NULL);
  *s = '\0';
}


VarStr::VarStr(const VarStr& other) :
  sz(other.sz),
  valFlag(other.valFlag)
{
  assert((s = (char*) malloc(sz)) != NULL);
  memcpy(s, other.s, sz);
}


VarStr& VarStr::operator = (const VarStr& other)
{
  *s = '\0';
  append(other, strlen(other));
  return *this;
}


VarStr& VarStr::set(const char* _s, int len)
{
  *s = '\0';
  append(_s, len);
  return *this;
}

VarStr& VarStr::append(const char* _s, int len)
{
  const int    _slen = strlen(_s);
  const size_t extra = len < 0 ? _slen : (_slen < len ? _slen : len);
  const size_t newsz = strlen(s) + extra + 1;

  if (newsz > sz) {
    assert((s = (char*) realloc(s, newsz)) != NULL);
    sz = newsz;
  }

  if (len < 0)
    strcat(s, _s);
  else
    strncat(s, _s, len);

  valFlag = 1;
  return *this;
}

VarStr& VarStr::expand(size_t _sz)
{
  sz += _sz;
  assert((s = (char*) realloc(s, sz)) != NULL);
  return *this;
}

void VarStr::substitute(char from, char to)
{
  for (char* p = s; *p; p++) {
    if (*p == from) *p = to;
  }
}

