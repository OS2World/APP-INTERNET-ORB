// orb - HTML preprocessor
// strassoc.hpp - string associative array class header
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


#ifndef _STRASSOC_HPP
#define _STRASSOC_HPP

#include <stddef.h>

class StringAssoc
{
  public:
    StringAssoc();
    virtual ~StringAssoc();

    virtual const char* value(const char* key, const char* val = NULL);
    virtual int         remove(const char* key);
    virtual void        clear();
    virtual void        validate() const;
    virtual void        dump() const;

    virtual StringAssoc& operator += (const StringAssoc& other);

  protected:
    class Cell
    {
      public:
        Cell(void);
        Cell(Cell* _prev, const char* _key, const char* _val);
        ~Cell();

        const char* key() const { return keyp; };
        const char* val(const char* v = NULL);

        Cell*       prev(Cell* p = NULL);
        Cell*       next(Cell* n = NULL);

      private:
        char* const keyp;
        char*       valp;

        Cell* prevp;
        Cell* nextp;
    };

    Cell* root;
};

#endif
