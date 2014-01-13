// orb - HTML preprocessor
// varstr.hpp - variable-length string class header
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


#ifndef _VARSTR_HPP
#define _VARSTR_HPP

#include <stddef.h>
#include <stdlib.h>

class VarStr {
  public:
    VarStr(const char* _s = NULL, int len = -1);
    VarStr(size_t _sz);
    VarStr(const VarStr& other);

    virtual ~VarStr() { free(s); }

    VarStr& operator = (const VarStr& other);

    VarStr& set(   const char* _s, int len = -1);
    VarStr& append(const char* _s, int len = -1);
    VarStr& expand(size_t _sz);

    VarStr& clear()   { *s = '\0'; valFlag = 1; return *this; }

    operator       char* ()       { return s; }
    operator const char* () const { return s; }

    const char& operator [] (size_t n) const { return s[n]; }
    char&       operator [] (size_t n)       { return s[n]; }

    void substitute(char from, char to);

    int isValid() const { return valFlag; }
    void valid(int _v)  { valFlag = _v;   }

  private:
    enum { Block = 64 };

    char*  s;
    size_t sz;
    int    valFlag;
};

#endif


